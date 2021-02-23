#include "stats.h"
#include <chrono>
#include <thread>
#include "app.h"

void Stats::print() const noexcept {
//    auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
//            std::chrono::steady_clock::now().time_since_epoch()
//    ).count();
//    auto timeElapsed = currentTime - startTime_.load();
    int64_t rps{0};
//    if (timeElapsed > 0) {
//        rps = requestsCnt_.load() * 1000 / timeElapsed;
//    }

    infof("Requests count: %lld", requestsCnt_.load());
    infof("RPS: %lld", rps);
//    infof("Time elapsed: %lld", timeElapsed);
}

Stats::Stats() {
//    startTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
//            std::chrono::steady_clock::now().time_since_epoch()
//    ).count();
}

void statsPrintLoop() {
    for (auto i = 0; i < 1 << 30; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));

        getApp().getStats().print();
    }
}
