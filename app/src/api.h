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

enum class ApiEndpointType : int {
    CheckHealth = 0,
    Explore = 1,
};

class Request {

public:
    ApiEndpointType type_{0};
    std::variant<Area> request_;

    Request() = default;

    Request(const Request &r) = delete;

    Request(Request &&r) = default;

    Request &operator=(const Request &r) = delete;

    Request &operator=(Request &&r) = default;

    Area getExploreRequest() const noexcept {
        return std::get<Area>(request_);
    }

    static Request NewCheckHealthRequest() noexcept {
        Request r{};
        r.type_ = ApiEndpointType::CheckHealth;
        return r;
    }

    static Request NewExploreRequest(Area area) noexcept {
        Request r{};
        r.type_ = ApiEndpointType::Explore;
        r.request_ = area;
        return r;
    }
};

class Response {
public:

    using HealthResponseWrapper = Expected<HttpResponse<HealthResponse>>;
    using ExploreResponseWrapper = Expected<HttpResponse<ExploreResponse>>;

    Response(const Response &r) = delete;

    Response(Response &&r) = default;

    Response &operator=(const Response &r) = delete;

    Response &operator=(Response &&r) = default;

    ApiEndpointType type_;
    std::variant<HealthResponseWrapper, ExploreResponseWrapper> response_;


    explicit Response(Expected<HttpResponse<HealthResponse>> &&healthResponse)
            : type_{ApiEndpointType::CheckHealth},
              response_{healthResponse} {}

    explicit Response(ExploreResponseWrapper &&r) : type_{ApiEndpointType::Explore}, response_{r} {}

    [[nodiscard]] HealthResponseWrapper getHealthResponse() const noexcept {
        return std::get<HealthResponseWrapper>(response_);
    }
};


class Api {
private:
    std::vector<std::thread> threads_;

    std::mutex requestsMu_;
    std::condition_variable requestCondVar_;
    std::list<Request> requests_;

    std::mutex responsesMu_;
    std::list<Response> responses_;
    std::condition_variable responsesCondVar_;


    std::string address_;

    void threadLoop() noexcept;

    static Expected<Response> makeApiRequest(HttpClient &client, const Request &r) noexcept;

    void publishResponse(Response &&r) noexcept;

    ExpectedVoid scheduleRequest(Request r) noexcept;

public:
    explicit Api(size_t threadsCount, std::string address);

    Api(const Api &o) = delete;

    Api(Api &&o) = delete;

    Api &operator=(const Api &o) = delete;

    Api &operator=(Api &&o) = delete;

    ~Api();

    ExpectedVoid scheduleCheckHealth() noexcept;

    ExpectedVoid explore(Area area) noexcept;

    std::optional<Response> getAvailableResponse() noexcept;


};

#endif //HIGHLOADCUP2021_API_H
