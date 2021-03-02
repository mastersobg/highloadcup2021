#ifndef HIGHLOADCUP2021_APP_H
#define HIGHLOADCUP2021_APP_H

#include "stats.h"
#include "api.h"
#include <atomic>
#include <thread>


class App {
private:
    std::thread statsThread_;
    std::string address_;
    Api api_;
    Stats stats_{};

    std::atomic<bool> stopped_{false};

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
