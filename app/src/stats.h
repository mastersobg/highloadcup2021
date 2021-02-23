#ifndef HIGHLOADCUP2021_STATS_H
#define HIGHLOADCUP2021_STATS_H


#include <cstdint>
#include <atomic>
#include "log.h"
#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <mutex>

struct EndpointStats {
    std::map<int32_t, int32_t> httpCodes;
    std::vector<int32_t> durations;
};

class Stats {
private:
    std::atomic<int64_t> requestsCnt_{0};
    std::atomic<int64_t> curlErrCnt_{0};


    std::mutex endpointStatsMutex_;
    std::unordered_map<std::string, EndpointStats> endpointStatsMap_;

    std::atomic<int64_t> lastTickRequestsCnt_{0};
    std::atomic<int64_t> startTime_{0};

    std::atomic<bool> stopped_{false};

    void printEndpointsStats();

public:

    Stats();

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

    void recordEndpointStats(const std::string &endpoint, int32_t httpCode, int32_t durationMs) noexcept;

    void stop() noexcept;

    bool isStopped() const noexcept;

    void print() noexcept;
};

void statsPrintLoop();

#endif //HIGHLOADCUP2021_STATS_H
