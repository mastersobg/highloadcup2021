#ifndef HIGHLOADCUP2021_APP_H
#define HIGHLOADCUP2021_APP_H

#include "stats.h"


class App {
private:
    Stats stats_{};

public:
    App() {}

    App(const App &o) = delete;

    App(App &&o) = delete;

    App &operator=(const App &o) = delete;

    App &operator=(App &&o) = delete;

    Stats &getStats() noexcept {
        return stats_;
    }
};

App &getApp();

#endif //HIGHLOADCUP2021_APP_H
