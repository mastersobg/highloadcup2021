#include "app.h"
#include <curl/curl.h>
#include <csignal>
#include <cstdlib>
#include <thread>
#include <chrono>

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

App::App() : statsThread_{statsPrintLoop},
             address_{std::getenv("ADDRESS")},
             api_{10, address_} {
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
    curl_global_cleanup();
    statsThread_.join();
}

std::pair<int16_t, int16_t> App::fireInitExplores() noexcept {
    for (int16_t i = 0; i < (int16_t) kFieldMaxX; i++) {
        for (int16_t j = 0; j < (int16_t) kFieldMaxY; j++) {
            auto err = api_.scheduleExplore(Area(i, j, 1, 1));
            if (err.hasError()) {
                return {i, j};
            }
        }
    }
    return {kFieldMaxX, kFieldMaxY};
}

void App::run() noexcept {
    fireInitExplores();

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
        default: {
            errorf("unknown response type: %d", resp.getType());
            break;
        }
    }
    return ErrorCode::kUnknownRequestType;
}

ExpectedVoid App::processExploreResponse(Request &req, HttpResponse<ExploreResponse> &resp) noexcept {
    if (resp.getHttpCode() != 200) {
        return api_.scheduleExplore(req.getExploreRequest());
    }
    auto successResp = std::move(resp).getResponse();
    getStats().recordExploreCell(successResp.amount_);

    return NoErr;
}
