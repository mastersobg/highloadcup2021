#include "app.h"
#include <curl/curl.h>
#include <csignal>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <memory>
#include <limits>
#include <cassert>

App app{};

void printBuildInfo() {
#ifndef BUILD_TYPE
#define BUILD_TYPE "unknown"
#endif

#ifndef COMMIT_HASH
#define COMMIT_HASH "unknown"
#endif

    infof("Build type: %s commit hash: %s", BUILD_TYPE, COMMIT_HASH);
}

App &getApp() {
    return app;
}

App::App() :
        stopped_{false},
        statsThread_{statsPrintLoop},
        address_{std::getenv("ADDRESS")},
        api_{kApiThreadCount, address_},
        rateLimiter_{kMaxRPS} {
    printBuildInfo();
    if (auto val = curl_global_init(CURL_GLOBAL_ALL)) {
        errorf("curl global init failed: %d", val);
        throw std::runtime_error("curl init failed");
    }
    std::signal(SIGINT, []([[maybe_unused]]int signal) {
        app.stop();
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        std::abort();
    });
}

App::~App() {
    stop();
    curl_global_cleanup();
    statsThread_.join();
}

ExpectedVoid App::fireInitRequests() noexcept {
    for (size_t i = 0; i < kMaxLicensesCount; i++) {
        if (auto err = scheduleIssueLicense(); err.hasError()) {
            return err;
        }
    }
    auto root = ExploreArea::NewExploreArea(nullptr, Area(0, 0, kFieldMaxX, kFieldMaxY), 0.0, 0,
                                            kTreasuriesCount);
    state_.setRootExploreArea(root);
    createSubAreas(root);

    for (size_t i = 0; i < kExploreConcurrentRequestsCnt; i++) {
        if (auto err = api_.scheduleExplore(state_.fetchNextExploreArea()); err.hasError()) {
            return err;
        }
    }
    return NoErr;
}

void App::run() noexcept {
    if (auto err = fireInitRequests(); err.hasError()) {
        errorf("fireInitRequests: error: %d", err.error());
        return;
    }

    for (;;) {
        if (getApp().isStopped()) {
            break;
        }

        auto response = api_.getAvailableResponse();

        auto err = processResponse(response);
        if (err.hasError()) {
            errorf("error occurred: %d", err.error());
            break;
        }

        getStats().recordInUseLicenses(state_.getInUseLicensesCount());
        getStats().recordCoinsAmount(state_.getCoinsAmount());
    }

}

ExpectedVoid App::processResponse(Response &resp) noexcept {
    switch (resp.getType()) {
        case ApiEndpointType::Explore: {
            if (resp.getExploreResponse().hasError()) {
                return resp.getExploreResponse().error();
            }
            auto apiResp = resp.getExploreResponse().get();
            return processExploreResponse(resp.getRequest(), apiResp);
        }
        case ApiEndpointType::IssueFreeLicense: {
            if (resp.getIssueLicenseResponse().hasError()) {
                return resp.getIssueLicenseResponse().error();
            }
            auto apiResp = resp.getIssueLicenseResponse().get();
            return processIssueLicenseResponse(resp.getRequest(), apiResp);
        }
        case ApiEndpointType::Dig: {
            if (resp.getDigResponse().hasError()) {
                return resp.getDigResponse().error();
            }
            auto apiResp = resp.getDigResponse().get();
            return processDigResponse(resp.getRequest(), apiResp);
        }
        case ApiEndpointType::Cash: {
            if (resp.getCashResponse().hasError()) {
                return resp.getCashResponse().error();
            }
            auto apiResp = resp.getCashResponse().get();
            return processCashResponse(resp.getRequest(), apiResp);
        }
        case ApiEndpointType::IssuePaidLicense: {
            if (resp.getIssueLicenseResponse().hasError()) {
                return resp.getIssueLicenseResponse().error();
            }
            auto apiResp = resp.getIssueLicenseResponse().get();
            return processIssueLicenseResponse(resp.getRequest(), apiResp);
        }
        default: {
            errorf("unknown response type: %d", resp.getType());
            break;
        }
    }
    return ErrorCode::kUnknownRequestType;
}

ExpectedVoid
App::processExploredArea(ExploreAreaPtr &exploreArea, size_t actualTreasuriesCnt, bool removeFromQueue) noexcept {
    if (exploreArea->explored_) {
        getStats().incDuplicateSetExplored();
        return NoErr;
    }
    if (removeFromQueue) {
        state_.removeExploreAreaFromQueue(exploreArea);
    }
    getStats().incExploredArea(exploreArea->area_.getArea());
    exploreArea->actualTreasuriesCnt_ = actualTreasuriesCnt;
    exploreArea->explored_ = true;

    if (exploreArea->actualTreasuriesCnt_ > 0 && exploreArea->area_.getArea() == 1) {
        getStats().recordTreasuriesCnt((int) exploreArea->actualTreasuriesCnt_);
        state_.setLeftTreasuriesAmount(exploreArea->area_.posX_, exploreArea->area_.posY_,
                                       (int32_t) exploreArea->actualTreasuriesCnt_);
        if (auto err = scheduleDigRequest(exploreArea->area_.posX_,
                                          exploreArea->area_.posY_,
                                          1); err.hasError()) {
            return err.error();
        }
    }

    if (exploreArea->area_.getArea() > 1 && exploreArea->actualTreasuriesCnt_ > 0) {
        createSubAreas(exploreArea);
    }

    return NoErr;
}

ExpectedVoid App::processExploreResponse(Request &req, HttpResponse<ExploreResponse> &resp) noexcept {
    if (resp.getHttpCode() != 200) {
        return api_.scheduleExplore(req.getExploreRequest());
    }
    auto successResp = std::move(resp).getResponse();
    auto exploreArea = req.getExploreRequest();

    processExploredArea(exploreArea, successResp.amount_, false);

    size_t exploredTreasuriesCnt{0};
    size_t nonExploredAreasCnt{0};
    size_t nonExploredChildren{0};
    for (const auto &child : exploreArea->parent_->children_) {
        if (child->explored_) {
            exploredTreasuriesCnt += child->actualTreasuriesCnt_;
        } else {
            nonExploredChildren++;
            nonExploredAreasCnt += child->area_.getArea();
        }
    }

    auto leftTreasuriesCnt = exploreArea->parent_->actualTreasuriesCnt_ - exploredTreasuriesCnt;
    for (auto &v : exploreArea->parent_->children_) {
        if (!v->explored_) {
            v->expectedTreasuriesCnt_ = (double) leftTreasuriesCnt / (double) nonExploredAreasCnt;
        }
    }

    if (nonExploredChildren == 1) {
        for (auto &child : exploreArea->parent_->children_) {
            if (!child->explored_) {
                processExploredArea(child, leftTreasuriesCnt, true);
                break;
            }
        }
    }

#ifdef _HLC_DEBUG
    assert(state_.hasMoreExploreAreas());
#endif


    auto ea = state_.fetchNextExploreArea();
    return api_.scheduleExplore(ea);
}

ExpectedVoid App::processIssueLicenseResponse([[maybe_unused]]Request &req, HttpResponse<License> &resp) noexcept {
    if (resp.getHttpCode() >= 400 && resp.getHttpCode() < 500) {
        auto errResp = std::move(resp).getErrResponse();
        errorf("processIssueLicenseResponse: err code: %d err message: %s", errResp.errorCode_,
               errResp.message_.c_str());
        return ErrorCode::kIssueLicenseError;
    }
    if (resp.getHttpCode() != 200) {
        return scheduleIssueLicense();
    }

    auto license = std::move(resp).getResponse();
    state_.addLicence(license);
    getStats().incIssuedLicenses();

    for (; state_.hasQueuedDigRequests();) {
        if (!state_.hasAvailableLicense()) {
            break;
        }

        auto r = state_.getNextDigRequest();
        if (auto err = scheduleDigRequest(r.x_, r.y_, r.depth_); err.hasError()) {
            return err.error();
        }
    }
    return NoErr;
}

ExpectedVoid App::scheduleIssueLicense() noexcept {
    if (state_.hasCoins()) {
        if (auto err = api_.scheduleIssuePaidLicense(state_.borrowCoin()); err.hasError()) {
            return err.error();
        }
    } else {
        if (auto err = api_.scheduleIssueFreeLicense(); err.hasError()) {
            return err.error();
        }
    }
    return NoErr;
}

ExpectedVoid App::processDigResponse(Request &req, HttpResponse<std::vector<TreasureID>> &resp) noexcept {
    auto digRequest = req.getDigRequest();
    if (resp.getHttpCode() == 200 || resp.getHttpCode() == 404) {
        auto &license = state_.getLicenseById(digRequest.licenseId_);
        license.digConfirmed_++;
        if (license.digAllowed_ == license.digConfirmed_) {
            if (auto err = scheduleIssueLicense(); err.hasError()) {
                return err.error();
            }
        }
    }
    switch (resp.getHttpCode()) {
        case 200: {
            auto treasuries = std::move(resp).getResponse();
            getStats().recordTreasureDepth(digRequest.depth_, (int) treasuries.size());
            for (const auto &id : treasuries) {
                if (state_.getCoinsAmount() > kCashSkipThreshold) {
                    getStats().incCashSkippedCnt();
                    continue;
                }
                if (auto err = api_.scheduleCash(id, digRequest.depth_); err.hasError()) {
                    return err.error();
                }
            }

            auto leftCount = state_.getLeftTreasuriesAmount(digRequest.posX_, digRequest.posY_);
            state_.setLeftTreasuriesAmount(digRequest.posX_, digRequest.posY_, leftCount - (int32_t) treasuries.size());
            leftCount = state_.getLeftTreasuriesAmount(digRequest.posX_, digRequest.posY_);
            if (leftCount < 0) {
                return ErrorCode::kTreasuriesLeftInconsistency;
            }
            if (leftCount > 0) {
                return scheduleDigRequest(digRequest.posX_, digRequest.posY_, (int8_t) (digRequest.depth_ + 1));
            }

            return NoErr;
        }
        case 404: {
            return scheduleDigRequest(digRequest.posX_, digRequest.posY_, (int8_t) (digRequest.depth_ + 1));
        }
        default: {
            auto httpCode = resp.getHttpCode();
            auto apiErr = std::move(resp).getErrResponse();
            errorf("unexpected dig response: http code: %d api code: %d message: %s", httpCode, apiErr.errorCode_,
                   apiErr.message_.c_str());
            return ErrorCode::kUnexpectedDigResponse;
        }
    }
}

ExpectedVoid App::processCashResponse(Request &r, HttpResponse<Wallet> &resp) noexcept {
    auto httpCode = resp.getHttpCode();
    if (httpCode >= 500) {
        return api_.scheduleCash(r.getCashRequest().treasureId_, r.getCashRequest().depth_);
    }
    if (httpCode >= 400) {
        auto apiErr = std::move(resp).getErrResponse();
        errorf("unexpected cash response: http code: %d api code: %d message: %s", httpCode, apiErr.errorCode_,
               apiErr.message_.c_str());
        return ErrorCode::kUnexpectedCashResponse;
    }
    auto successResp = std::move(resp).getResponse();
    state_.addCoins(successResp);
    getStats().incCashedCoins((int64_t) successResp.coins.size());
    getStats().recordCoinsDepth(r.getCashRequest().depth_, (int) successResp.coins.size());
    return NoErr;
}

ExpectedVoid App::scheduleDigRequest(int16_t x, int16_t y, int8_t depth) noexcept {
    if (state_.hasAvailableLicense()) {
        auto licenseId = state_.reserveAvailableLicenseId();
        if (licenseId.hasError()) {
            return licenseId.error();
        }
        return api_.scheduleDig({licenseId.get(), x, y, depth});
    } else {
        state_.addDigRequest({x, y, depth});
        return NoErr;
    }
}

void App::createSubAreas(const ExploreAreaPtr &root) noexcept {
    auto h = kExploreAreas[root->exploreDepth_].height;
    auto w = kExploreAreas[root->exploreDepth_].width;
    auto x1 = root->area_.posX_;
    auto x2 = root->area_.posX_ + root->area_.sizeX_;
    auto y1 = root->area_.posY_;
    auto y2 = root->area_.posY_ + root->area_.sizeY_;
    for (int i = x1; i < x2; i += h) {
        for (int j = y1; j < y2; j += w) {
            auto curH = h;
            auto curW = w;
            if (i + curH > x2) {
                curH = (int16_t) (x2 - i);
            }
            if (j + curW > y2) {
                curW = (int16_t) (y2 - j);
            }
            auto ea = ExploreArea::NewExploreArea(
                    root,
                    Area((int16_t) i, (int16_t) j, curH, curW),
                    (double) root->actualTreasuriesCnt_ / (double) (root->area_.getArea()),
                    root->exploreDepth_ + 1,
                    0
            );
            root->addChild(ea);
            state_.addExploreArea(ea);
        }
    }
}
