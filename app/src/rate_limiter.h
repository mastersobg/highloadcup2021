#ifndef HIGHLOADCUP2021_RATE_LIMITER_H
#define HIGHLOADCUP2021_RATE_LIMITER_H

#include <cstdlib>
#include <list>
#include <chrono>
#include <mutex>

struct RateLimiterEntity {
    int32_t cost_{0};
    std::chrono::time_point<std::chrono::steady_clock> ts_;

    RateLimiterEntity(int32_t cost, std::chrono::time_point<std::chrono::steady_clock> ts) : cost_{cost}, ts_{ts} {}
};

class RateLimiter {
private:
    int32_t maxRps_;
    std::list<RateLimiterEntity> requests_;
    int32_t totalCost_;
    std::mutex mu_;

public:
    RateLimiter() = delete;

    explicit RateLimiter(int32_t rps);

    RateLimiter(const RateLimiter &o) = delete;

    RateLimiter(RateLimiter &&o) = delete;

    RateLimiter &operator=(const RateLimiter &o) = delete;

    RateLimiter &operator=(RateLimiter &&o) = delete;


    void acquire(int32_t cost);

};


#endif //HIGHLOADCUP2021_RATE_LIMITER_H
