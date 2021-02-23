#include "stats.h"
#include <chrono>
#include <thread>
#include "app.h"

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


    lastTickRequestsCnt_ = requestsCnt_.load();
}

void Stats::stop() noexcept {
    stopped_ = true;
}

bool Stats::isStopped() const noexcept {
    return stopped_;
}

void statsPrintLoop() {
    for (;;) {
        std::this_thread::sleep_for(std::chrono::milliseconds(statsSleepDelayMs));

        if (getApp().getStats().isStopped()) {
            break;
        }

        getApp().getStats().print();
    }
}
