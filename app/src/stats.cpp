#include "stats.h"
#include <chrono>
#include <thread>
#include "app.h"
#include "util.h"
#include <algorithm>
#include "sys.h"
#include <numeric>

constexpr int64_t statsSleepDelayMs = 5000;

void Stats::print() noexcept {
    auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
    ).count();

    auto timeElapsedMs = currentTime - startTime_.load();
    int64_t rps{0};
    if (timeElapsedMs > 0) {
        rps = requestsCnt_.load() * 1000L / timeElapsedMs;
    }
    int64_t tickRPS = (requestsCnt_.load() - lastTickRequestsCnt_.load()) * 1000L / statsSleepDelayMs;

    infof("Requests count: %lld", requestsCnt_.load());
    infof("RPS: %lld", rps);
    infof("Tick RPS: %lld", tickRPS);
    infof("Curl errs: %lld", curlErrCnt_.load());
    infof("Time elapsed: %lld ms", timeElapsedMs);
    infof("Timeouts: %d", timeoutCnt_.load());
    infof("Explored area: %lld", exploredArea_.load());
    infof("Explored treasuries amount: %lld", treasuriesCnt_.load());
    infof("Cash skipped: %lld", cashSkippedCnt_.load());
    infof("Duplicate set explored: %lld", duplicateSetExplored_.load());
    infof("Total process time: %lld expore time: %lld", totalProcessResponseTime_.load(),
          totalProcessExploreResponseTime_.load());
    infof("Avg explore request cost: %f",
          (double) exploreRequestTotalCost_.load() / (double) exploreRequestsCnt_.load());
    infof("Treasures per second: %f", (double) treasuriesCnt_.load() / (double) timeElapsedMs * 1000.0);
    if (treasuriesCnt_.load() > 0) {
        infof("Avg explore request per treasure: %f",
              (double) exploreRequestsCnt_.load() / (double) treasuriesCnt_.load());
        infof("Avg explored area per treasure: %f",
              (double) exploreRequestTotalArea_.load() / (double) treasuriesCnt_.load());
        infof("Avg cost per treasure: %f", (double) exploreRequestTotalCost_.load() / (double) treasuriesCnt_.load());
    }
    infof("Total requests duration: %lld", totalRequestsDuration_.load());
//    infof("Woken with empty requests queue: %lld", wokenWithEmptyRequestsQueue_.load());
//    infof("Average in use licenses: %f", (double) inUseLicensesSum_ / (double) inUseLicensesCnt_);
//    infof("Average in flight requests: %f", (double) inFlightRequestsSum_ / (double) inFlightRequestsCnt_);
//    infof("Average in flight explore requests: %f",
//          (double) inFlightExploreRequestsSum_ / (double) inFlightExploreRequestsCnt_);
    infof("Total cashed: %lld coins, %lld treasuries, %f avg", cashedCoinsSum_.load(), cashedTreasuriesCnt_.load(),
          (double) cashedCoinsSum_.load() / (double) cashedTreasuriesCnt_.load());
    infof("Issued licenses: %lld", issuedLicenses_.load());
    infof("Coins amount: %d", coinsAmount_.load());

    printEndpointsStats();
    printDepthHistogram();
    printTreasuriesDiggedCount();

    printCoinsDepthHistogram();
//    printExploreAreaHistogram();
    printCpuStat();

    lastTickRequestsCnt_ = requestsCnt_.load();
}

Stats::Stats() {
    startTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
    ).count();
}

void Stats::recordEndpointStats(const std::string &endpoint, int32_t httpCode, int64_t durationMcs) noexcept {
    std::scoped_lock lck{endpointStatsMutex_};

    auto &stats = endpointStatsMap_[endpoint];
    stats.httpCodes[httpCode]++;
    stats.durations.push_back(durationMcs);
    totalRequestsDuration_ += durationMcs;
}

void Stats::printEndpointsStats() noexcept {
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

        auto avg = std::accumulate(stats.durations.begin(), stats.durations.end(), (int64_t) 0) /
                   (int64_t) stats.durations.size();
        logString += "\nduration: ";
        logString += "avg: ";
        writeIntToString(avg, logString);
        std::sort(stats.durations.begin(), stats.durations.end());
        auto size = stats.durations.size();
        logString += " p50: ";
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

void Stats::printDepthHistogram() noexcept {
    std::shared_lock lock(depthHistogramMutex_);

    std::string logString{};
    for (size_t i = 0; i <= 10; i++) {
        writeIntToString(depthHistogram_[i], logString);
        logString += ", ";
    }
    infof("depth histogram: %s", logString.c_str());
}

void Stats::printCoinsDepthHistogram() noexcept {
    std::shared_lock lock(depthCoinsHistogramMutex_);
    std::string logString{};
    int64_t totalDigs{0};
    for (size_t i = 0; i <= 10; i++) {
        writeIntToString(depthCoinsHistogram_[i], logString);
        logString += ", ";

        totalDigs += (int64_t) i * depthCoinsHistogram_[i];
    }

    infof("coin avg dig count: %f", (double) totalDigs / (double) cashedCoinsSum_.load());
    infof("depth coins histogram: %s", logString.c_str());
}

void Stats::printCpuStat() noexcept {
    auto stats = getCpuStats();
    infof("CPU active: %f%% CPU idle: %f%%\nUser: %f%% Nice: %f%% System: %f%% Idle: %f%% IOWait: %f%% Irq: %f%% SoftIrq: %f%% Steal: %f%% Guest: %f%% GuestNice: %f%%",
          stats.getActivePercent(), stats.getIdlePercent(),
          stats.getStatsPercent(CpuStatsState::User),
          stats.getStatsPercent(CpuStatsState::Nice),
          stats.getStatsPercent(CpuStatsState::System),
          stats.getStatsPercent(CpuStatsState::Idle),
          stats.getStatsPercent(CpuStatsState::IOWait),
          stats.getStatsPercent(CpuStatsState::Irq),
          stats.getStatsPercent(CpuStatsState::Softirq),
          stats.getStatsPercent(CpuStatsState::Steal),
          stats.getStatsPercent(CpuStatsState::Guest),
          stats.getStatsPercent(CpuStatsState::GuestNice)
    );
}

void Stats::printExploreAreaHistogram() noexcept {
    std::shared_lock lock(exploreAreaHistogramMutex_);

    std::string avgDurationStr{};
    std::string countStr{};
    for (size_t i = 0; i < exploreAreaHistogramCount_.size(); i++) {
        int64_t avgDuration = 0;
        if (exploreAreaHistogramCount_[i] > 0) {
            avgDuration = exploreAreaHistogramDuration_[i] / exploreAreaHistogramCount_[i];
        }
        writeIntToString(avgDuration, avgDurationStr);
        avgDurationStr += ", ";

        writeIntToString(exploreAreaHistogramCount_[i], countStr);
        countStr += ", ";
    }
    infof("explore area avg duration histogram: %s", avgDurationStr.c_str());
    infof("explore area count histogram: %s", countStr.c_str());
}

void Stats::printTreasuriesDiggedCount() noexcept {
    std::shared_lock lock(depthHistogramMutex_);

    int cnt{0};
    for (size_t i = 0; i <= 10; i++) {
        cnt += depthHistogram_[i];
    }
    infof("Digged treasuries count: %d", cnt);

}

const std::vector<int64_t> exploreCostThresholds = {
        0,
        789, // 1
        999, // 2
        1023, // 4
        1539, // 8
        2051, // 16
        2578, // 32
        3108,//64
        3692, //128
        4378, // 256
        5307, //512
        6772, // 1024
        11163, //2048
        12210, //4096
        13443, //8192
        15289, //16384
        1125899906842624,
        1125899906842624,
        1125899906842624,
        1125899906842624,
        1125899906842624,
        1125899906842624,
        1125899906842624,
        1125899906842624,
        1125899906842624,
        1125899906842624,
        1125899906842624,
        1125899906842624,
        1125899906842624,
        1125899906842624,
        1125899906842624,
        1125899906842624
};

int64_t Stats::calculateExploreCost(int64_t area) noexcept {
    int64_t pos = 0;
    for (pos = 0; (int64_t) 1LL << pos <= area; pos++) {
    }
    return exploreCostThresholds[static_cast<size_t>(pos)];
}


void statsPrintLoop() {
//    std::thread t{recordInFlightRequests};
    for (;;) {
        std::this_thread::sleep_for(std::chrono::milliseconds(statsSleepDelayMs));

        getApp().getStats().print();

        if (getApp().isStopped()) {
            break;
        }
    }
}

