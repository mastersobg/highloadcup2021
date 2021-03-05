#ifndef HIGHLOADCUP2021_APP_H
#define HIGHLOADCUP2021_APP_H

#include "stats.h"
#include "api.h"
#include <atomic>
#include <thread>
#include <utility>
#include "state.h"
#include <vector>


class App {
private:
    std::atomic<bool> stopped_;

    std::thread statsThread_;
    std::string address_;
    Api api_;
    Stats stats_{};
    State state_;

    [[nodiscard]] ExpectedVoid fireInitRequests() noexcept;

    [[nodiscard]] ExpectedVoid processResponse(Response &r) noexcept;

    ExpectedVoid processExploreResponse(Request &req, HttpResponse<ExploreResponse> &resp) noexcept;

    ExpectedVoid processIssueLicenseResponse(Request &req, HttpResponse<License> &resp) noexcept;

    ExpectedVoid processDigResponse(Request &req, HttpResponse<std::vector<TreasureID>> &resp) noexcept;

    ExpectedVoid processCashResponse(Request &r, HttpResponse<Wallet> &resp) noexcept;

    Expected<LicenseID> reserveLicense() noexcept;

public:
    App();

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

    Stats &getStats() noexcept {
        return stats_;
    }

    void run() noexcept;
};

App &getApp();

#endif //HIGHLOADCUP2021_APP_H
