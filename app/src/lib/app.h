#ifndef HIGHLOADCUP2021_APP_H
#define HIGHLOADCUP2021_APP_H

#include "stats.h"
#include "api.h"
#include <atomic>
#include <thread>
#include <utility>
#include "state.h"
#include "rate_limiter.h"
#include <vector>
#include <memory>

class App {
private:
    std::atomic<bool> stopped_{false};

    std::shared_ptr<Log> log_;
    std::shared_ptr<Api> api_;
    std::shared_ptr<Stats> stats_;
    State state_;
//    RateLimiter rateLimiter_;


    [[nodiscard]] ExpectedVoid fireInitRequests() noexcept;

    [[nodiscard]] ExpectedVoid processResponse(Response &r) noexcept;

    [[nodiscard]]ExpectedVoid processExploreResponse(Request &req, HttpResponse<ExploreResponse> &resp) noexcept;

    [[nodiscard]]ExpectedVoid processIssueLicenseResponse(Request &req, HttpResponse<License> &resp) noexcept;

    [[nodiscard]]ExpectedVoid processDigResponse(Request &req, HttpResponse<std::vector<TreasureID>> &resp) noexcept;

    [[nodiscard]]ExpectedVoid processCashResponse(Request &r, HttpResponse<Wallet> &resp) noexcept;

    [[nodiscard]]ExpectedVoid scheduleIssueLicense() noexcept;

    [[nodiscard]]ExpectedVoid scheduleDigRequest(int16_t x, int16_t y, int8_t depth) noexcept;

    [[nodiscard]]ExpectedVoid
    processExploredArea(ExploreAreaPtr exploreArea, size_t actualTreasuriesCnt) noexcept;

    [[nodiscard]] ExpectedVoid createSubAreas(const ExploreAreaPtr &root) noexcept;

public:
    App(std::shared_ptr<Api> api, std::shared_ptr<Stats> stats, std::shared_ptr<Log> log);

    ~App();

    App(const App &o) = delete;

    App(App &&o) = delete;

    App &operator=(const App &o) = delete;

    App &operator=(App &&o) = delete;

    void stop() noexcept {
        stopped_ = true;
    }

    bool isStopped() const noexcept {
        return stopped_.load();
    }

    void run() noexcept;

    static std::shared_ptr<App> createApp();
};

#endif //HIGHLOADCUP2021_APP_H
