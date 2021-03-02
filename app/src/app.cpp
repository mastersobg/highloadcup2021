#include "app.h"
#include <curl/curl.h>
#include <csignal>

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

App::App() : statsThread_{statsPrintLoop}, address_{std::getenv("ADDRESS")}, api_{10, address_} {
    printBuildInfo();
    if (auto val = curl_global_init(CURL_GLOBAL_ALL)) {
        errorf("curl global init failed: %d", val);
        throw std::runtime_error("curl init failed");
    }
    std::signal(SIGINT, []([[maybe_unused]]int signal) {
        app.stop();
    });
}

App::~App() {
    curl_global_cleanup();
    statsThread_.join();
}

void App::run() noexcept {
    for (;;) {
        if (getApp().isStopped()) {
            break;
        }
        auto err = api_.explore(Area(0, 0, 1, 1));
        if (err.hasError()) {
            if (err.error() != ErrorCode::kMaxApiRequestsQueueSizeExceeded) {
                errorf("error on scheduling API request: %d", err.error());
            }
        }

        auto response = api_.getAvailableResponse();
        if (!response) {
            debugf("no available response");
            continue;
        }
        Response resp = std::move(response.value());
        switch (resp.type_) {
            case ApiEndpointType::CheckHealth: {
                auto healthResponseWrapper = resp.getHealthResponse();
                if (healthResponseWrapper.hasError()) {
                    errorf("response contains error: %d", healthResponseWrapper.error());
                    break;
//                    goto loopExit;
                }
                auto healthResponse = std::move(healthResponseWrapper).get();

//                debugf("%s", std::move(healthResponse).getResponse().details_.c_str());
                break;
            }
            case ApiEndpointType::Explore: {

                break;
            }
            default: {
                errorf("unknown response type: %d", resp.type_);
                break;
            }
        }

    }

}

