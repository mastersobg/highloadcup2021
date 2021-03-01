#ifndef HIGHLOADCUP2021_APP_H
#define HIGHLOADCUP2021_APP_H

#include "stats.h"
#include <atomic>


class App {
private:
    Stats stats_{};

    std::atomic<bool> stopped_{false};

public:
    App() {}

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
};

App &getApp();

#endif //HIGHLOADCUP2021_APP_H
