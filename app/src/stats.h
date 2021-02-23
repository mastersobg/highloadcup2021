#ifndef HIGHLOADCUP2021_STATS_H
#define HIGHLOADCUP2021_STATS_H


#include <cstdint>
#include <atomic>
#include "log.h"

class Stats {
private:
    std::atomic<int64_t> requestsCnt_{0};
    std::atomic<int64_t> startTime_{0};

public:

    Stats();

    Stats(const Stats &o) = delete;

    Stats(Stats &&o) = delete;

    Stats &operator=(const Stats &o) = delete;

    Stats &operator=(Stats &oo) = delete;

    void incRequestsCnt() noexcept {
        requestsCnt_++;
    }

    void print() const noexcept;
};

void statsPrintLoop();

#endif //HIGHLOADCUP2021_STATS_H
