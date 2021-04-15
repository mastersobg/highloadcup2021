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

    log_->info() << "Requests count: " << requestsCnt_.load();
    log_->info() << "RPS: " << rps;
    log_->info() << "Tick RPS: " << tickRPS;
    log_->info() << "Curl errs: " << curlErrCnt_.load();
    log_->info() << "Time elapsed: " << timeElapsedMs << " ms";
    log_->info() << "Timeouts: " << timeoutCnt_.load();
    log_->info() << "Explored area: " << exploredArea_.load();
    log_->info() << "Explored treasuries amount: " << treasuriesCnt_.load();
    log_->info() << "Cash skipped: " << cashSkippedCnt_.load();
    log_->info() << "Duplicate set explored: " << duplicateSetExplored_.load();
    log_->info() << "Total process time: " << totalProcessResponseTime_.load() <<
                 " explore time: " << totalProcessExploreResponseTime_.load();
    log_->info() << "Avg explore request cost: "
                 << (double) exploreRequestTotalCost_.load() / (double) exploreRequestsCnt_.load();
    log_->info() << "Treasures per second:" << (double) treasuriesCnt_.load() / (double) timeElapsedMs * 1000.0;
    if (treasuriesCnt_.load() > 0) {
        log_->info() << "Avg explore request per treasure: " <<
                     (double) exploreRequestsCnt_.load() / (double) treasuriesCnt_.load();
        log_->info() << "Avg explored area per treasure: " <<
                     (double) exploreRequestTotalArea_.load() / (double) treasuriesCnt_.load();
        log_->info() << "Avg cost per treasure: "
                     << (double) exploreRequestTotalCost_.load() / (double) treasuriesCnt_.load();
    }
    log_->info() << "Total requests duration: " << totalRequestsDuration_.load();
//    infof("Woken with empty requests queue: %lld", wokenWithEmptyRequestsQueue_.load());
//    infof("Average in use licenses: %f", (double) inUseLicensesSum_ / (double) inUseLicensesCnt_);
//    infof("Average in flight requests: %f", (double) inFlightRequestsSum_ / (double) inFlightRequestsCnt_);
//    infof("Average in flight explore requests: %f",
//          (double) inFlightExploreRequestsSum_ / (double) inFlightExploreRequestsCnt_);
    log_->info() << "Total cashed: " << cashedCoinsSum_.load() << " coins, " << cashedTreasuriesCnt_.load()
                 << " treasuries, " << (double) cashedCoinsSum_.load() / (double) cashedTreasuriesCnt_.load() << " avg";
    log_->info() << "Issued licenses: " << issuedLicenses_.load();
    log_->info() << "Coins amount: " << coinsAmount_.load();

    printEndpointsStats();
    printDepthHistogram();
    printTreasuriesDiggedCount();

    printCoinsDepthHistogram();
//    printExploreAreaHistogram();
//    printCpuStat();

    lastTickRequestsCnt_ = requestsCnt_.load();
}

Stats::Stats(std::shared_ptr<Log> log) : log_{std::move(log)} {
    startTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
    ).count();

    statsThread_ = std::thread(&Stats::statsPrintLoop, this);
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

        log_->info() << logString.c_str();
    }
}

void Stats::printDepthHistogram() noexcept {
    std::shared_lock lock(depthHistogramMutex_);

    std::string logString{};
    for (size_t i = 0; i <= 10; i++) {
        writeIntToString(depthHistogram_[i], logString);
        logString += ", ";
    }
    log_->info() << "depth histogram: " << logString.c_str();
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

    log_->info() << "coin avg dig count: " << (double) totalDigs / (double) cashedCoinsSum_.load();
    log_->info() << "depth coins histogram: " << logString.c_str();
}

//void Stats::printCpuStat() noexcept {
//    auto stats = getCpuStats();
//    infof("CPU active: %f%% CPU idle: %f%%\nUser: %f%% Nice: %f%% System: %f%% Idle: %f%% IOWait: %f%% Irq: %f%% SoftIrq: %f%% Steal: %f%% Guest: %f%% GuestNice: %f%%",
//          stats.getActivePercent(), stats.getIdlePercent(),
//          stats.getStatsPercent(CpuStatsState::User),
//          stats.getStatsPercent(CpuStatsState::Nice),
//          stats.getStatsPercent(CpuStatsState::System),
//          stats.getStatsPercent(CpuStatsState::Idle),
//          stats.getStatsPercent(CpuStatsState::IOWait),
//          stats.getStatsPercent(CpuStatsState::Irq),
//          stats.getStatsPercent(CpuStatsState::Softirq),
//          stats.getStatsPercent(CpuStatsState::Steal),
//          stats.getStatsPercent(CpuStatsState::Guest),
//          stats.getStatsPercent(CpuStatsState::GuestNice)
//    );
//}

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
    log_->info() << "explore area avg duration histogram: " << avgDurationStr.c_str();
    log_->info() << "explore area count histogram: " << countStr.c_str();
}

void Stats::printTreasuriesDiggedCount() noexcept {
    std::shared_lock lock(depthHistogramMutex_);

    int cnt{0};
    for (size_t i = 0; i <= 10; i++) {
        cnt += depthHistogram_[i];
    }
    log_->info() << "Digged treasuries count: " << cnt;

}

const std::vector<int64_t> exploreCostThresholds = {
        0,
        1, // 1
        1, // 2
        2, // 4
        3, // 8
        4, // 16
        5, // 32
        6, // 64
        7, // 128
        8, // 256
        9, // 512
        10, // 1024
        11, // 2048
        12, // 4096
        13, // 8192
        14, // 16384
        15, // 32768
        16, // 65536
        17, // 131072
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


void Stats::statsPrintLoop() {
    for (;;) {
        std::this_thread::sleep_for(std::chrono::milliseconds(statsSleepDelayMs));

        print();

        if (shouldStopStatsThread_) {
            break;
        }
    }
}

Stats::~Stats() {
    shouldStopStatsThread_ = true;
    statsThread_.join();
}
