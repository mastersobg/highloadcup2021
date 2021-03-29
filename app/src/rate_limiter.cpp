#include "rate_limiter.h"
#include "log.h"
#include <thread>


const std::chrono::seconds Second(1);
const std::chrono::microseconds lastRequestDelay(50000);
const std::chrono::microseconds minLastRequestDelaySleep(500);
const std::chrono::milliseconds sleepMs(100);

RateLimiter::RateLimiter(int32_t rps) : maxRps_{rps}, totalCost_{0} {}

void RateLimiter::acquire([[maybe_unused]]int32_t cost) {
    std::scoped_lock lock(mu_);

    std::this_thread::sleep_for(sleepMs);
//    auto now = std::chrono::steady_clock::now();
//
//    while (!requests_.empty()) {
//        auto r = requests_.front();
//        if (now - r.ts_ <= Second) {
//            break;
//        }
//        totalCost_ -= r.cost_;
//        requests_.pop_front();
//    }
//
//    if (totalCost_ + cost > maxRps_) {
//        int32_t sum{0};
//        for (const auto &request : requests_) {
//            sum += request.cost_;
//            if (totalCost_ - sum + cost <= maxRps_) {
//                auto d = request.ts_ + Second - now;
//                std::this_thread::sleep_for(d);
//                break;
//            }
//        }
//    }
//    now = std::chrono::steady_clock::now();
//    if (!requests_.empty()) {
//        const auto &last = requests_.back();
//        auto sleepTime = last.ts_ + lastRequestDelay - now;
//        if (sleepTime > minLastRequestDelaySleep) {
////            infof("sleep for %d", sleepTime);
//            std::this_thread::sleep_for(sleepTime);
//        }
//    }
//
//    now = std::chrono::steady_clock::now();
//    requests_.emplace_back(cost, now);
//    totalCost_ += cost;
}
