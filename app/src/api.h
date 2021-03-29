#ifndef HIGHLOADCUP2021_API_H
#define HIGHLOADCUP2021_API_H

#include <list>
#include <thread>
#include <utility>
#include <vector>
#include <list>
#include <mutex>
#include <condition_variable>
#include "http_client.h"
#include <string>
#include <memory>
#include <variant>
#include <optional>
#include <atomic>
#include "api_entities.h"
#include <set>

enum class ApiEndpointType : int {
    CheckHealth = 0,
    Explore = 1,
    IssueFreeLicense = 2,
    IssuePaidLicense = 3,
    Dig = 4,
    Cash = 5,
};

struct CashRequest {
    TreasureID treasureId_;
    int8_t depth_;

    CashRequest(TreasureID treasureId, int8_t depth) : treasureId_(std::move(treasureId)), depth_(depth) {}
};

class Request {
    int8_t priority{0};
    int32_t cost_{100};
public:
    ApiEndpointType type_{0};
    std::variant<ExploreAreaPtr, CoinID, DigRequest, CashRequest> request_;

    Request() = default;

    Request(const Request &r) = delete;

    Request(Request &&r) = default;

    Request &operator=(const Request &r) = delete;

    Request &operator=(Request &&r) = default;

    ExploreAreaPtr getExploreRequest() const noexcept {
        return std::get<ExploreAreaPtr>(request_);
    }

    int32_t getCost() const noexcept {
        return cost_;
    }

    CoinID getIssueLicenseRequest() const noexcept {
        return std::get<CoinID>(request_);
    }

    DigRequest getDigRequest() const noexcept {
        return std::get<DigRequest>(request_);
    }

    [[nodiscard]] const CashRequest &getCashRequest() const noexcept {
        return std::get<CashRequest>(request_);
    }

    static Request NewCheckHealthRequest() noexcept {
        Request r{};
        r.priority = 0;
        r.type_ = ApiEndpointType::CheckHealth;
        return r;
    }

    static Request NewExploreRequest(ExploreAreaPtr area) noexcept {
        Request r{};
        r.priority = 1;
        r.type_ = ApiEndpointType::Explore;
        r.request_ = area;
        return r;
    }

    static Request NewIssuePaidLicenseRequest(CoinID coinId) noexcept {
        Request r{};
        r.priority = 3;
        r.type_ = ApiEndpointType::IssuePaidLicense;
        r.request_ = coinId;
        return r;
    }

    static Request NewDigRequest(DigRequest digRequest) noexcept {
        Request r{};
        r.priority = 2;
        r.type_ = ApiEndpointType::Dig;
        r.request_ = digRequest;
        return r;
    }

    static Request NewCashRequest(TreasureID id, int8_t depth) noexcept {
        Request r{};
        r.priority = 4;
        r.type_ = ApiEndpointType::Cash;
        r.request_ = CashRequest(std::move(id), depth);
        return r;
    }


    static Request NewIssueFreeLicenseRequest() noexcept {
        Request r{};
        r.priority = 3;
        r.type_ = ApiEndpointType::IssueFreeLicense;
        return r;
    }

    bool operator<(const Request &r2) const {
        return priority > r2.priority;
    }

};

class Response {
    using HealthResponseWrapper = Expected<HttpResponse<HealthResponse>>;
    using ExploreResponseWrapper = Expected<HttpResponse<ExploreResponse>>;
    using IssueLicenseWrapper = Expected<HttpResponse<License>>;
    using DigResponseWrapper = Expected<HttpResponse<std::vector<TreasureID>>>;
    using CashResponseWrapper = Expected<HttpResponse<Wallet>>;

    std::variant<HealthResponseWrapper, ExploreResponseWrapper, IssueLicenseWrapper,
            DigResponseWrapper, CashResponseWrapper> response_;
    Request request_;
public:


    Response(const Response &r) = delete;

    Response(Response &&r) = default;

    Response &operator=(const Response &r) = delete;

    Response &operator=(Response &&r) = default;

    [[nodiscard]] ApiEndpointType getType() const noexcept {
        return request_.type_;
    }

    Request &getRequest() noexcept {
        return request_;
    }

    Response(Request &&r, Expected<HttpResponse<Wallet>> &&cashResponse) :
            response_{std::move(cashResponse)},
            request_{std::move(r)} {}

    Response(Request &&r, Expected<HttpResponse<std::vector<TreasureID>>> &&digResponse) :
            response_{std::move(digResponse)},
            request_{std::move(r)} {}

    Response(Request &&r, Expected<HttpResponse<License>> &&licenseResponse) :
            response_{std::move(licenseResponse)},
            request_{std::move(r)} {
    }

    Response(Request &&r, Expected<HttpResponse<HealthResponse>> &&healthResponse)
            : response_{std::move(healthResponse)},
              request_{std::move(r)} {}

    Response(Request &&r, ExploreResponseWrapper &&exploreResponse) :
            response_{std::move(exploreResponse)},
            request_{std::move(r)} {}

    [[nodiscard]] HealthResponseWrapper &getHealthResponse() noexcept {
        return std::get<HealthResponseWrapper>(response_);
    }

    [[nodiscard]] ExploreResponseWrapper &getExploreResponse() noexcept {
        return std::get<ExploreResponseWrapper>(response_);
    }

    [[nodiscard]]IssueLicenseWrapper &getIssueLicenseResponse() noexcept {
        return std::get<IssueLicenseWrapper>(response_);
    }

    [[nodiscard]] DigResponseWrapper &getDigResponse() noexcept {
        return std::get<DigResponseWrapper>(response_);
    }

    [[nodiscard]] CashResponseWrapper &getCashResponse() noexcept {
        return std::get<CashResponseWrapper>(response_);
    }

};


class Api {
private:
    std::vector<std::thread> threads_;

    std::mutex requestsMu_;
    std::condition_variable requestCondVar_;
    std::multiset<Request> requests_;

    std::mutex responsesMu_;
    std::list<Response> responses_;
    std::condition_variable responsesCondVar_;

    std::atomic<int64_t> inFlightRequestsCnt_{0};
    std::atomic<int64_t> inFlightExploreRequestsCnt_{0};

    std::string address_;

    void threadLoop();

    Expected<Response> makeApiRequest(HttpClient &client, Request &r) noexcept;

    void publishResponse(Response &&r) noexcept;

public:
    explicit Api(size_t threadsCount, std::string address);

    Api(const Api &o) = delete;

    Api(Api &&o) = delete;

    Api &operator=(const Api &o) = delete;

    Api &operator=(Api &&o) = delete;

    ~Api();

    ExpectedVoid scheduleCheckHealth() noexcept;

    ExpectedVoid scheduleRequest(Request r) noexcept;

    ExpectedVoid scheduleExplore(ExploreAreaPtr area) noexcept;

    ExpectedVoid scheduleIssueFreeLicense() noexcept;

    ExpectedVoid scheduleIssuePaidLicense(CoinID coinId) noexcept;

    ExpectedVoid scheduleDig(DigRequest r) noexcept;

    ExpectedVoid scheduleCash(TreasureID id, int8_t depth) noexcept;

    Response getAvailableResponse() noexcept;

    size_t requestsQueueSize() noexcept;

    int64_t getInFlightRequestsCnt() const noexcept {
        return inFlightRequestsCnt_;
    }

    int64_t getInFlightExploreRequestsCnt() const noexcept {
        return inFlightExploreRequestsCnt_;
    }
};

#endif //HIGHLOADCUP2021_API_H
