#include "app.h"
#include <curl/curl.h>
#include <csignal>
#include <chrono>
#include <memory>
#include <limits>
#include <cassert>

App::App(std::shared_ptr<Api> api, std::shared_ptr<Stats> stats, std::shared_ptr<Log> log) :
        log_{std::move(log)},
        api_{std::move(api)},
        stats_{std::move(stats)} {
#ifndef BUILD_TYPE
#define BUILD_TYPE "unknown"
#endif

#ifndef COMMIT_HASH
#define COMMIT_HASH "unknown"
#endif

    log_->info() << "Build type: " << BUILD_TYPE << " commit hash: " << COMMIT_HASH;
    if (auto val = curl_global_init(CURL_GLOBAL_ALL)) {
        log_->error() << "curl global init failed: " << val;
        throw std::runtime_error("curl init failed");
    }
}

App::~App() {
    stop();
    curl_global_cleanup();
}

ExpectedVoid App::fireInitRequests() noexcept {
    for (size_t i = 0; i < kMaxLicensesCount; i++) {
        if (auto err = scheduleIssueLicense(); err.hasError()) {
            return err;
        }
    }
    auto root = ExploreArea::NewExploreArea(nullptr, Area(0, 0, kFieldMaxX, kFieldMaxY), 0,
                                            kTreasuriesCount);
    state_.setRootExploreArea(root);
    if (auto err = createSubAreas(root); err.hasError()) {
        return err.error();
    }

    for (size_t i = 0; i < kExploreConcurrentRequestsCnt; i++) {
        if (auto err = api_->scheduleExplore(state_.fetchNextExploreArea()); err.hasError()) {
            return err;
        }
    }
    return NoErr;
}

void App::run() noexcept {
    if (auto err = fireInitRequests(); err.hasError()) {
        log_->error() << "fireInitRequests: error: " << err.error();
        return;
    }

    for (;;) {
        if (isStopped()) {
            break;
        }

        auto response = api_->getAvailableResponse();

        Measure<std::chrono::nanoseconds> tm;
        auto err = processResponse(response);
        stats_->addProcessResponseTime(tm.getInt64());
        if (err.hasError()) {
            if (err.error() != ErrorCode::kErrCurlTimeout) {
                log_->error() << "error occurred: " << err.error();
                break;
            } else {
                if (auto errInner = api_->scheduleRequest(std::move(response.getRequest())); errInner.hasError()) {
                    log_->error() << "error occurred: " << errInner.error();
                    break;
                }
                stats_->incTimeoutCnt();
            }
        }

        stats_->recordInUseLicenses(state_.getInUseLicensesCount());
        stats_->recordCoinsAmount(state_.getCoinsAmount());
    }

}

ExpectedVoid App::processResponse(Response &resp) noexcept {
    switch (resp.getType()) {
        case ApiEndpointType::Explore: {
            if (resp.getExploreResponse().hasError()) {
                return resp.getExploreResponse().error();
            }
            auto apiResp = resp.getExploreResponse().get();
            Measure<std::chrono::nanoseconds> tm;
            auto err = processExploreResponse(resp.getRequest(), apiResp);
            stats_->addProcessExploreResponseTime(tm.getInt64());
            return err;
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
            log_->error() << "unknown response type: " << resp.getType();
            break;
        }
    }
    return ErrorCode::kUnknownRequestType;
}

ExpectedVoid
App::processExploredArea(ExploreAreaPtr exploreArea, size_t actualTreasuriesCnt) noexcept {
#ifdef _HLC_DEBUG
    auto xBefore = exploreArea->area_.posX_;
    auto yBefore = exploreArea->area_.posY_;
#endif
    if (exploreArea->explored_) {
        stats_->incDuplicateSetExplored();
        return NoErr;
    }
    stats_->incExploredArea(exploreArea->area_.getArea());
    exploreArea->actualTreasuriesCnt_ = actualTreasuriesCnt;
    exploreArea->explored_ = true;
    state_.removeExploreAreaFromQueue(exploreArea->parent_);
    exploreArea->parent_->updateChildExplored(exploreArea);
    if (exploreArea->parent_->getLeftTreasuriesCnt() > 0) {
        state_.addExploreArea(exploreArea->parent_);
    }
#ifdef _HLC_DEBUG
    assert(exploreArea->area_.posX_ == xBefore && exploreArea->area_.posY_ == yBefore);
#endif

    if (exploreArea->actualTreasuriesCnt_ > 0 && exploreArea->area_.getArea() == 1) {
        stats_->recordTreasuriesCnt((int) exploreArea->actualTreasuriesCnt_);
        state_.setLeftTreasuriesAmount(exploreArea->area_.posX_, exploreArea->area_.posY_,
                                       (int32_t) exploreArea->actualTreasuriesCnt_);
        if (auto err = scheduleDigRequest(exploreArea->area_.posX_,
                                          exploreArea->area_.posY_,
                                          1); err.hasError()) {
            return err.error();
        }
    }

    if (exploreArea->area_.getArea() > 1 && exploreArea->actualTreasuriesCnt_ > 0) {
        if (auto err = createSubAreas(exploreArea); err.hasError()) {
            return err.error();
        }
    }

    return NoErr;
}

ExpectedVoid App::processExploreResponse(Request &req, HttpResponse<ExploreResponse> &resp) noexcept {
    if (resp.getHttpCode() != 200) {
        return api_->scheduleExplore(req.getExploreRequest());
    }
    auto successResp = std::move(resp).getResponse();
    auto exploreArea = req.getExploreRequest();

    if (auto err = processExploredArea(exploreArea, successResp.amount_); err.hasError()) {
        return err.error();
    }

    if (exploreArea->parent_->getNonExploredChildrenCnt() == 1) {
        auto child = exploreArea->parent_->getLastNonExploredChild();
        if (auto err = processExploredArea(child, exploreArea->parent_->getLeftTreasuriesCnt()); err.hasError()) {
            return err.error();
        }
    }

#ifdef _HLC_DEBUG
    assert(state_.hasMoreExploreAreas());
#endif

    return api_->scheduleExplore(state_.fetchNextExploreArea());
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
    stats_->incIssuedLicenses();

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
        if (auto err = api_->scheduleIssuePaidLicense(state_.borrowCoin()); err.hasError()) {
            return err.error();
        }
    } else {
        if (auto err = api_->scheduleIssueFreeLicense(); err.hasError()) {
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
            stats_->recordTreasureDepth(digRequest.depth_, (int) treasuries.size());
            for (const auto &id : treasuries) {
                if (digRequest.depth_ >= minDepthToCash) {
                    if (auto err = api_->scheduleCash(id, digRequest.depth_); err.hasError()) {
                        return err.error();
                    }
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
            log_->error() << "unexpected dig response: http code: " << httpCode << " api code: " << apiErr.errorCode_
                          << " message: " << apiErr.message_;
            return ErrorCode::kUnexpectedDigResponse;
        }
    }
}

ExpectedVoid App::processCashResponse(Request &r, HttpResponse<Wallet> &resp) noexcept {
    auto httpCode = resp.getHttpCode();
    if (httpCode >= 500) {
        return api_->scheduleCash(r.getCashRequest().treasureId_, r.getCashRequest().depth_);
    }
    if (httpCode >= 400) {
        auto apiErr = std::move(resp).getErrResponse();
        log_->error() << "unexpected cash response: http code: " << httpCode << " api code: " << apiErr.errorCode_
                      << " message: " << apiErr.message_;
        return ErrorCode::kUnexpectedCashResponse;
    }
    auto successResp = std::move(resp).getResponse();
    state_.addCoins(successResp);
    stats_->incCashedCoins((int64_t) successResp.coins.size());
    stats_->recordCoinsDepth(r.getCashRequest().depth_, (int) successResp.coins.size());
    return NoErr;
}

ExpectedVoid App::scheduleDigRequest(int16_t x, int16_t y, int8_t depth) noexcept {
    if (state_.hasAvailableLicense()) {
        auto licenseId = state_.reserveAvailableLicenseId();
        if (licenseId.hasError()) {
            return licenseId.error();
        }
        return api_->scheduleDig({licenseId.get(), x, y, depth});
    } else {
        state_.addDigRequest({x, y, depth});
        return NoErr;
    }
}

ExpectedVoid App::createSubAreas(const ExploreAreaPtr &root) noexcept {
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
                    root->exploreDepth_ + 1,
                    0
            );
            root->addChild(ea);
        }
    }
    if (root->getNonExploredChildrenCnt() == 1) {
        auto child = root->getLastNonExploredChild();
        if (auto err = processExploredArea(child, root->getLeftTreasuriesCnt()); err.hasError()) {
            return err.error();
        }
    } else {
        state_.addExploreArea(root);
    }
    return NoErr;
}

std::shared_ptr<App> App::createApp() {
    auto log = std::make_shared<Log>();
    auto stats = std::make_shared<Stats>(log);
    auto api = std::make_shared<Api>(stats, log);
    auto app = std::make_shared<App>(api, stats, log);

    return app;
}
