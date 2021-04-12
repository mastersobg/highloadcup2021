#ifndef HIGHLOADCUP2021_HTTP_CLIENT_H
#define HIGHLOADCUP2021_HTTP_CLIENT_H

#include <string>
#include <curl/curl.h>
#include "api_entities.h"
#include <variant>
#include "error.h"
#include "const.h"
#include <chrono>
#include "stats.h"

template<class T>
class HttpResponse {
    std::variant<T, ApiError> resp_;
    int32_t httpCode_{0};
    std::chrono::microseconds latencyMcs_;
public:

    HttpResponse(ApiError &&err, int32_t httpCode, std::chrono::microseconds latencyMcs) :
            resp_{std::move(err)},
            httpCode_{httpCode},
            latencyMcs_{latencyMcs} {}

    HttpResponse(T &&r, int32_t httpCode, std::chrono::microseconds latencyMcs) :
            resp_{std::move(r)},
            httpCode_{httpCode},
            latencyMcs_{latencyMcs} {}

    [[nodiscard]] bool hasError() const noexcept {
        return std::holds_alternative<ApiError>(resp_);
    }

    [[nodiscard]] ApiError getErrResponse() &&{
        return std::move(std::get<ApiError>(resp_));
    }

    [[nodiscard]] T getResponse() &&{
        return std::move(std::get<T>(resp_));
    }

    [[nodiscard]] int32_t getHttpCode() const noexcept {
        return httpCode_;
    }

    [[nodiscard]] std::chrono::microseconds getLatencyMcs() const noexcept {
        return latencyMcs_;
    }

};

struct respHolder {
    std::string data;
};

class HttpClient {
    std::shared_ptr<Stats> stats_;

    CURL *session_;

    char errbuf_[CURL_ERROR_SIZE];
    JsonBufferType valueBuffer_[kJsonValueBufferCap];
    JsonBufferType parseBuffer_[kJsonParseBufferCap];
    respHolder resp_;
    std::string postDataBuffer_;

    const std::string baseURL_;
    const std::string checkHealthURL_;
    const std::string exploreURL_;
    const std::string cashURL_;
    const std::string digURL_;
    const std::string issueLicenseURL_;
    curl_slist *headers_;

    [[nodiscard]]Expected<int32_t> makeRequest(const std::string &url, const char *data) noexcept;

public:
    HttpClient(std::shared_ptr<Stats> stats, const std::string &address,
               const std::string &port, const std::string &schema);

    HttpClient(const HttpClient &c) = delete;

    HttpClient(HttpClient &&c) = delete;

    HttpClient &operator=(const HttpClient &c) = delete;

    HttpClient &operator=(HttpClient &&c) = delete;

    ~HttpClient();

    [[nodiscard]] Expected<HttpResponse<HealthResponse>> checkHealth() noexcept;

    [[nodiscard]] Expected<HttpResponse<ExploreResponse>> explore(const Area &a) noexcept;

    [[nodiscard]] Expected<HttpResponse<Wallet>> cash(const TreasureID &treasureId) noexcept;

    [[nodiscard]] Expected<HttpResponse<std::vector<TreasureID>>> dig(DigRequest request) noexcept;

    [[nodiscard]] Expected<HttpResponse<License>> issueLicense(CoinID coinId) noexcept;

    [[nodiscard]] Expected<HttpResponse<License>> issueFreeLicense() noexcept;
};


#endif //HIGHLOADCUP2021_HTTP_CLIENT_H
