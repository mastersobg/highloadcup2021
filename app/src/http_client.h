#ifndef HIGHLOADCUP2021_HTTP_CLIENT_H
#define HIGHLOADCUP2021_HTTP_CLIENT_H

#include <string>
#include <curl/curl.h>
#include "api_entities.h"
#include <variant>
#include "error.h"
#include "const.h"

template<class T>
class HttpResponse {
    std::variant<T, ApiError> resp_;
    int32_t httpCode_;

public:

    HttpResponse(ApiError &&err, int32_t httpCode) : resp_{err}, httpCode_{httpCode} {}

    HttpResponse(T &&r, int32_t httpCode) : resp_{r}, httpCode_{httpCode} {}

    [[nodiscard]] bool hasError() const noexcept {
        return std::holds_alternative<ApiError>(resp_);
    }

    [[nodiscard]] ApiError getErrResponse() &&{
        return std::move(std::get<ApiError>(resp_));
    }

    [[nodiscard]] T getResponse() &&{
        return std::move(std::get<T>(resp_));
    }

};

struct respHolder {
    std::string data;
};

class HttpClient {
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
    HttpClient(const std::string &address, const std::string &port, const std::string &schema);

    HttpClient(const HttpClient &c) = delete;

    HttpClient(HttpClient &&c) = delete;

    HttpClient &operator=(const HttpClient &c) = delete;

    HttpClient &operator=(HttpClient &&c) = delete;

    ~HttpClient();

    [[nodiscard]] Expected<HttpResponse<HealthResponse>> checkHealth() noexcept;

    [[nodiscard]] Expected<HttpResponse<ExploreResponse>> explore(const Area &a) noexcept;

    [[nodiscard]] Expected<HttpResponse<void *>> cash(const TreasureID &treasureId, Wallet &buf) noexcept;

    [[nodiscard]] Expected<HttpResponse<void *>>
    dig(LicenseID licenseId, int16_t posX, int16_t posY, int8_t depth, std::vector<TreasureID> &buf) noexcept;

    [[nodiscard]] Expected<HttpResponse<License>> issueLicense(const Wallet &w) noexcept;
};


#endif //HIGHLOADCUP2021_HTTP_CLIENT_H
