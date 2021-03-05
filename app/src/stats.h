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
#include <array>
#include <shared_mutex>

struct EndpointStats {
    std::map<int32_t, int32_t> httpCodes;
    std::vector<int32_t> durations;
};

class Stats {
private:
    std::atomic<int64_t> requestsCnt_{0};
    std::atomic<int64_t> curlErrCnt_{0};
    std::atomic<int64_t> exploreCellTotalAmount_{0};
    std::atomic<int64_t> exploreCellCount_{0};
    std::atomic<int64_t> cellsWithTreasuries{0};
    std::atomic<int64_t> wokenWithEmptyRequestsQueue_{0};
    std::atomic<int64_t> licensesSum_{0};
    std::atomic<int64_t> licensesCnt_{0};
    std::atomic<size_t> coinsAmount_{0};


    std::mutex endpointStatsMutex_;
    std::unordered_map<std::string, EndpointStats> endpointStatsMap_;

    std::shared_mutex depthHistogramMutex_;
    std::array<int, 11> depthHistogram_{0,};


    std::shared_mutex depthCoinsHistogramMutex_;
    std::array<int, 11> depthCoinsHistogram_{0,};

    std::shared_mutex exploreAreaHistogramMutex_;
    std::array<int64_t, 10> exploreAreaHistogramCount_{0,};
    std::array<int64_t, 10> exploreAreaHistogramDuration_{0,};

    std::atomic<int64_t> lastTickRequestsCnt_{0};
    std::atomic<int64_t> startTime_{0};

    void printEndpointsStats() noexcept;

    void printDepthHistogram() noexcept;

    void printCoinsDepthHistogram() noexcept;

    void printExploreAreaHistogram() noexcept;

    void printCpuStat() noexcept;

public:

    Stats();

    Stats(const Stats &o) = delete;

    Stats(Stats &&o) = delete;

    Stats &operator=(const Stats &o) = delete;

    Stats &operator=(Stats &oo) = delete;

    void incRequestsCnt() noexcept {
        requestsCnt_++;
    }

    void incWokenWithEmptyRequestsQueue() noexcept {
        wokenWithEmptyRequestsQueue_++;
    }

    void incCurlErrCnt() noexcept {
        curlErrCnt_++;
    }

    void recordLicenses(int cnt) noexcept {
        licensesSum_ += cnt;
        licensesCnt_++;
    }

    void recordTreasureDepth(int depth, int count) noexcept {
        std::scoped_lock lock(depthHistogramMutex_);

        depthHistogram_[(size_t) depth] += count;
    }

    void recordCoinsDepth(int depth, int coinsCount) noexcept {
        std::scoped_lock lock(depthCoinsHistogramMutex_);

        depthCoinsHistogram_[(size_t) depth] += coinsCount;
    }

    void recordEndpointStats(const std::string &endpoint, int32_t httpCode, int32_t durationMs) noexcept;

    void recordExploreCell(uint32_t amount) noexcept {
        exploreCellTotalAmount_ += (int64_t) amount;
        exploreCellCount_++;
        if (amount > 0) {
            cellsWithTreasuries++;
        }
    }

    void recordCoinsAmount(size_t amount) {
        coinsAmount_ = amount;
    }

    void recordExploreArea(int area, int32_t durationMs) noexcept {
        size_t idx = 0;
        while (area >= 10) {
            area /= 10;
            idx++;
        }

        std::scoped_lock lock(exploreAreaHistogramMutex_);
        exploreAreaHistogramCount_[idx]++;
        exploreAreaHistogramDuration_[idx] += durationMs;
    }

    void print() noexcept;
};

void statsPrintLoop();

#endif //HIGHLOADCUP2021_STATS_H
