#include "api.h"
#include "app.h"
#include <stdexcept>
#include "log.h"
#include <utility>
#include "error.h"

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

Expected<Response> Api::makeApiRequest(HttpClient &client, const Request &r) noexcept {
    switch (r.type) {
        case ApiEndpointType::CheckHealth: {
            auto resp = client.checkHealth();
            return Response(std::move(resp));
        }
        default: {
            errorf("Unsupported request type: %d", r.type);
            return ErrorCode::kUnknownRequestType;
        }
    }
}

void Api::publishResponse(Response &&r) noexcept {
    std::scoped_lock lock(responsesMu_);

    responses_.push_back(std::move(r));
    responsesCondVar_.notify_one();
}

ExpectedVoid Api::scheduleCheckHealth() noexcept {

    std::unique_lock lock(requestsMu_);

    auto size = requests_.size();

    if (size >= kMaxApiRequestsQueueSize) {
        return ErrorCode::kMaxApiRequestsQueueSizeExceeded;
    }

    requests_.push_back(Request::NewCheckHealthRequest());

    lock.unlock();
    requestCondVar_.notify_one();

    return NoErr;
}

std::optional<Response> Api::getAvailableResponse() noexcept {

    std::unique_lock lock(responsesMu_);
    responsesCondVar_.wait(lock, [this] {
        return !responses_.empty();
    });

    auto r = std::move(responses_.front());
    responses_.pop_front();
//    responsesSize_--;

    return r;
}

