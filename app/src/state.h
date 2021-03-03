#ifndef HIGHLOADCUP2021_STATE_H
#define HIGHLOADCUP2021_STATE_H

#include <cstdint>
#include <utility>
#include "const.h"

class State {
private:
    int16_t lastX_{0};
    int16_t lastY_{0};
public:
    State() = default;

    State(const State &s) = delete;

    State(State &&s) = delete;

    State &operator=(const State &s) = delete;

    State &operator=(State &&s) = delete;

    int16_t &lastX() noexcept {
        return lastX_;
    }

    int16_t &lastY() noexcept {
        return lastY_;
    }

    std::pair<int16_t, int16_t> nextExploreCoord() {
        auto x = lastX_;
        auto y = (int16_t) (lastY_ + 1);
        if (y == kFieldMaxY) {
            x++;
            y = 0;
        }

        lastX_ = x;
        lastY_ = y;
        return {x, y};
    }

};


#endif //HIGHLOADCUP2021_STATE_H
