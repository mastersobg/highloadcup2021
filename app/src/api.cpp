#include "api.h"
#include "app.h"
#include <stdexcept>
#include "log.h"
#include <utility>
#include "error.h"
#include <thread>

Api::Api(size_t threadsCount, std::string address) : address_{std::move(address)} {
    for (size_t i = 0; i < threadsCount; i++) {
        std::thread t(&Api::threadLoop, this);
        threads_.push_back(std::move(t));
    }
}

void Api::threadLoop() {
    HttpClient client{address_, "8000", "http"};
    for (;;) {
        std::unique_lock lock(requestsMu_);
        requestCondVar_.wait(lock, [this] {
            return getApp().isStopped() || !requests_.empty();
        });

        if (getApp().isStopped()) {
            break;
        }

        if (requests_.empty()) {
            errorf("Woken up but requests queue is empty");
            getApp().getStats().incWokenWithEmptyRequestsQueue();
            continue;
        }

        Request r = std::move(requests_.extract(requests_.begin()).value());
#ifdef _HLC_DEBUG
        if (r.type_ == ApiEndpointType::Dig) {
            for (const auto &it : requests_) {
                if (it.type_ == ApiEndpointType::Dig) {
                    const auto &req = it.getDigRequest();
                    if (req.depth_ > r.getDigRequest().depth_) {
                        assert(false);
                    }
                }
            }
        }
#endif

        lock.unlock();

        getApp().getRateLimiter().acquire(r.getCost());

        inFlightRequestsCnt_++;
        auto ret = makeApiRequest(client, r);
        inFlightRequestsCnt_--;
        if (ret.hasError()) {
            errorf("Error during making API request: %d", ret.error());
            throw std::runtime_error("Error during making API request");
        }

        publishResponse(std::move(ret).get());
    }
}

Api::~Api() {
    if (!getApp().isStopped()) {
        errorf("Api destructor was called while app is not stopped yet");
    }

    requestCondVar_.notify_all();

    for (auto &t : threads_) {
        t.join();
    }
}

Expected<Response> Api::makeApiRequest(HttpClient &client, Request &r) noexcept {
    switch (r.type_) {
        case ApiEndpointType::CheckHealth: {
            auto resp = client.checkHealth();
            return Response(std::move(r), std::move(resp));
        }
        case ApiEndpointType::Explore: {
            auto exploreRequest = r.getExploreRequest();
            inFlightExploreRequestsCnt_++;
            auto ret = client.explore(exploreRequest->area_);
            inFlightExploreRequestsCnt_--;
            return Response(std::move(r), std::move(ret));
        }
        case ApiEndpointType::IssueFreeLicense: {
            return Response(std::move(r), client.issueFreeLicense());
        }
        case ApiEndpointType::Dig: {
            auto digRequest = r.getDigRequest();
            return Response(std::move(r), client.dig(digRequest));
        }
        case ApiEndpointType::Cash: {
            auto cashRequest = r.getCashRequest();
            return Response(std::move(r), client.cash(cashRequest.treasureId_));
        }
        case ApiEndpointType::IssuePaidLicense: {
            auto coinId = r.getIssueLicenseRequest();
            return Response(std::move(r), client.issueLicense(coinId));
        }
        default: {
            errorf("Unsupported request type: %d", r.type_);
            return ErrorCode::kUnknownRequestType;
        }
    }
}

void Api::publishResponse(Response &&r) noexcept {
    std::unique_lock lock(responsesMu_);

    responses_.push_back(std::move(r));
    lock.unlock();
    responsesCondVar_.notify_one();
}

ExpectedVoid Api::scheduleCheckHealth() noexcept {
    return scheduleRequest(Request::NewCheckHealthRequest());
}

Response Api::getAvailableResponse() noexcept {
    std::unique_lock lock(responsesMu_);
    responsesCondVar_.wait(lock, [this] {
        return !responses_.empty();
    });

    auto r = std::move(responses_.front());
    responses_.pop_front();
    if (r.getType() == ApiEndpointType::Explore) {
        getApp().getStats().incInFlightExploreRequests(-1);
    }

    return r;
}

ExpectedVoid Api::scheduleExplore(ExploreAreaPtr area) noexcept {
    return scheduleRequest(Request::NewExploreRequest(std::move(area)));
}

ExpectedVoid Api::scheduleIssueFreeLicense() noexcept {
    return scheduleRequest(Request::NewIssueFreeLicenseRequest());
}

ExpectedVoid Api::scheduleIssuePaidLicense(CoinID coinId) noexcept {
    return scheduleRequest(Request::NewIssuePaidLicenseRequest(coinId));
}

ExpectedVoid Api::scheduleDig(DigRequest r) noexcept {
    return scheduleRequest(Request::NewDigRequest(r));
}


ExpectedVoid Api::scheduleCash(TreasureID id, int8_t depth) noexcept {
    return scheduleRequest(Request::NewCashRequest(std::move(id), depth));
}

ExpectedVoid Api::scheduleRequest(Request r) noexcept {
    std::unique_lock lock(requestsMu_);

    if (requests_.size() >= kMaxApiRequestsQueueSize) {
        return ErrorCode::kMaxApiRequestsQueueSizeExceeded;
    }

    if (r.type_ == ApiEndpointType::Explore) {
        getApp().getStats().incInFlightExploreRequests(1);
    }
    requests_.insert(std::move(r));

    lock.unlock();
    requestCondVar_.notify_one();
    return NoErr;
}

size_t Api::requestsQueueSize() noexcept {
    std::scoped_lock lock(requestsMu_);
    return requests_.size();
}

