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

void Api::threadLoop() noexcept {
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
        }


        Request r = std::move(requests_.front());
        requests_.pop_front();

        lock.unlock();

        auto ret = makeApiRequest(client, r);
        if (ret.hasError()) {
            errorf("Error during making API request: %d", ret.error());
            continue;
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
            auto area = r.getExploreRequest();
            return Response(std::move(r), client.explore(area));
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

    return r;
}

ExpectedVoid Api::scheduleExplore(Area area) noexcept {
    return scheduleRequest(Request::NewExploreRequest(area));
}

ExpectedVoid Api::scheduleRequest(Request r) noexcept {
    std::unique_lock lock(requestsMu_);

    if (requests_.size() >= kMaxApiRequestsQueueSize) {
        return ErrorCode::kMaxApiRequestsQueueSizeExceeded;
    }

    requests_.push_back(std::move(r));

    lock.unlock();
    requestCondVar_.notify_one();
    return NoErr;
}

size_t Api::requestsQueueSize() noexcept {
    std::scoped_lock lock(requestsMu_);
    return requests_.size();
}

