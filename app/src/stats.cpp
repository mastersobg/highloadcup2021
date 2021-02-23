#include "stats.h"
#include <chrono>
#include <thread>
#include "app.h"
#include "util.h"
#include <algorithm>

constexpr int64_t statsSleepDelayMs = 5000;

void Stats::print() noexcept {
    tickCnt_++;

    auto timeElapsedMs = tickCnt_.load() * statsSleepDelayMs;
    int64_t rps = requestsCnt_.load() * 1000L / timeElapsedMs;
    int64_t tickRPS = (requestsCnt_.load() - lastTickRequestsCnt_.load()) * 1000L / statsSleepDelayMs;

    infof("Requests count: %lld", requestsCnt_.load());
    infof("RPS: %lld", rps);
    infof("Tick RPS: %lld", tickRPS);
    infof("Curl errs: %lld", curlErrCnt_.load());
    infof("Time elapsed: %lld", timeElapsedMs);

    printEndpointsStats();

    lastTickRequestsCnt_ = requestsCnt_.load();
}

void Stats::stop() noexcept {
    stopped_ = true;
}

bool Stats::isStopped() const noexcept {
    return stopped_;
}

void Stats::recordEndpointStats(const std::string &endpoint, int32_t httpCode, int32_t durationMs) noexcept {
    std::scoped_lock lck{endpointStatsMutex_};

    auto &stats = endpointStatsMap_[endpoint];
    stats.httpCodes[httpCode]++;
    stats.durations.push_back(durationMs);
}

void Stats::printEndpointsStats() {
    std::scoped_lock endpointStatsLock(endpointStatsMutex_);
    for (auto &[endpointId, stats] : endpointStatsMap_) {
        std::string logString{};
        logString += "endpoint: " + endpointId + "\n";
        logString += "httpCodes: ";
        for (const auto &[httpCode, count]: stats.httpCodes) {
            writeIntToString(httpCode, logString);
            logString += ": ";
            writeIntToString(count, logString);
            logString += " ";
        }

        logString += "\nduration percentiles: ";
        std::sort(stats.durations.begin(), stats.durations.end());
        auto size = stats.durations.size();
        logString += "p50: ";
        writeIntToString(stats.durations[size / 2], logString);
        logString += " p90: ";
        writeIntToString(stats.durations[size * 9 / 10], logString);
        logString += " p95: ";
        writeIntToString(stats.durations[size * 95 / 100], logString);
        logString += " p99: ";
        writeIntToString(stats.durations[size * 99 / 100], logString);
        logString += " p999: ";
        writeIntToString(stats.durations[size * 999 / 1000], logString);
        logString += " p100: ";
        writeIntToString(stats.durations[size - 1], logString);

        infof("%s", logString.c_str());
    }
}

void statsPrintLoop() {
    for (;;) {
        std::this_thread::sleep_for(std::chrono::milliseconds(statsSleepDelayMs));

        getApp().getStats().print();

        if (getApp().getStats().isStopped()) {
            break;
        }
    }
}
