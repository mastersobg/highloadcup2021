#ifndef HIGHLOADCUP2021_APP_H
#define HIGHLOADCUP2021_APP_H

#include "stats.h"
#include "api.h"
#include <atomic>
#include <thread>
#include <utility>
#include "state.h"


class App {
private:
    std::thread statsThread_;
    std::string address_;
    Api api_;
    Stats stats_{};
    State state_;

    std::atomic<bool> stopped_{false};

    [[nodiscard]] ExpectedVoid fireInitExplores() noexcept;

    [[nodiscard]] ExpectedVoid processResponse(Response &r) noexcept;

    ExpectedVoid processExploreResponse(Request &req, HttpResponse<ExploreResponse> &resp) noexcept;

public:
    App();

    ~App();

    App(const App &o) = delete;

    App(App &&o) = delete;

    App &operator=(const App &o) = delete;

    App &operator=(App &&o) = delete;

    void stop() noexcept {
        stopped_ = true;
    }

    bool isStopped() const noexcept {
        return stopped_.load();
    }

    Stats &getStats() noexcept {
        return stats_;
    }

    void run() noexcept;
};

App &getApp();

#endif //HIGHLOADCUP2021_APP_H
