#ifndef HIGHLOADCUP2021_STATS_H
#define HIGHLOADCUP2021_STATS_H


#include <cstdint>
#include <atomic>
#include "log.h"

class Stats {
private:
    std::atomic<int64_t> requestsCnt_{0};
    std::atomic<int64_t> tickCnt_{0};
    std::atomic<int64_t> curlErrCnt_{0};

    std::atomic<int64_t> lastTickRequestsCnt_{0};

    std::atomic<bool> stopped_{false};

public:

    Stats() = default;

    Stats(const Stats &o) = delete;

    Stats(Stats &&o) = delete;

    Stats &operator=(const Stats &o) = delete;

    Stats &operator=(Stats &oo) = delete;

    void incRequestsCnt() noexcept {
        requestsCnt_++;
    }

    void incCurlErrCnt() noexcept {
        curlErrCnt_++;
    }

    void stop() noexcept;

    bool isStopped() const noexcept;

    void print() noexcept;
};

void statsPrintLoop();

#endif //HIGHLOADCUP2021_STATS_H
