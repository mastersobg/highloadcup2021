#include "http_client.h"
#include "log.h"
#include <stdexcept>
#include <curl/curl.h>
#include "json.h"
#include "app.h"
#include "util.h"
#include <chrono>

size_t httpCallback(char *ptr, size_t size, size_t nmemb, void *ud) noexcept {
    auto resp = (respHolder *) ud;
    for (size_t i = 0; i < nmemb; i++) {
        resp->data += ptr[i];
    }
    return size * nmemb;
}

template<class T, class Convertor>
Expected<HttpResponse<T>>
prepareResponse(Expected<int32_t> &response, std::string &data, std::chrono::microseconds latencyMcs,
                JsonBufferType *valueBuffer,
                JsonBufferType *parseBuffer, Convertor convert) {
    auto code = response.get();
    if (code == 200) {
        return HttpResponse<T>(convert(data), code, latencyMcs);
    } else {
        return HttpResponse<T>(unmarshalApiError(data, valueBuffer, parseBuffer), code, latencyMcs);
    }
}

HttpClient::HttpClient(std::shared_ptr<Stats> stats, const std::string &address,
                       const std::string &port,
                       const std::string &schema
) :
        stats_{std::move(stats)},
        errbuf_{0,},
        valueBuffer_{0,},
        parseBuffer_{0,},
        baseURL_{schema + "://" + address + ":" + port},
        checkHealthURL_{baseURL_ + "/health-check"},
        exploreURL_{baseURL_ + "/explore"},
        cashURL_{baseURL_ + "/cash"},
        digURL_{baseURL_ + "/dig"},
        issueLicenseURL_{baseURL_ + "/licenses"},
        headers_{nullptr} {

    session_ = curl_easy_init();
    if (session_ == nullptr) {
        throw std::runtime_error("failed to construct session_");
    }

    if (curl_easy_setopt(session_, CURLOPT_WRITEFUNCTION, httpCallback) != CURLE_OK) {
        throw std::runtime_error("failed to set CURLOPT_WRITEFUNCTION");
    }
    if (curl_easy_setopt(session_, CURLOPT_ERRORBUFFER, errbuf_) != CURLE_OK) {
        throw std::runtime_error("failed to set CURLOPT_ERRORBUFFER");
    }
    if (curl_easy_setopt(session_, CURLOPT_WRITEDATA, (void *) &this->resp_) != CURLE_OK) {
        throw std::runtime_error("failed to set CURLOPT_WRITEDATA");
    }
    if (curl_easy_setopt(session_, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4) != CURLE_OK) {
        throw std::runtime_error("failed to set CURLOPT_IPRESOLVE");
    }

    headers_ = curl_slist_append(headers_, "Content-Type: application/json");
    headers_ = curl_slist_append(headers_, "Accept:");

    if (curl_easy_setopt(session_, CURLOPT_HTTPHEADER, headers_) != CURLE_OK) {
        throw std::runtime_error("failed to set CURLOPT_HTTPHEADER");
    }

    if (curl_easy_setopt(session_, CURLOPT_TIMEOUT_MS, kRequestTimeout) != CURLE_OK) {
        throw std::runtime_error("failed to set CURLOPT_TIMEOUT_MS");
    }

//    curl_easy_setopt(session_, CURLOPT_VERBOSE, 1L);
}

HttpClient::~HttpClient() {
    curl_easy_cleanup(session_);
    curl_slist_free_all(headers_);
}

Expected<HttpResponse<HealthResponse>> HttpClient::checkHealth() noexcept {
    Measure<std::chrono::microseconds> tm;
    auto ret = makeRequest(checkHealthURL_, nullptr);
    if (ret.hasError()) {
        return ret.error();
    }
    auto latency = tm.getDuration();

    stats_->recordEndpointStats("health", ret.get(), latency.count());
    return prepareResponse<HealthResponse>(ret, resp_.data, latency, valueBuffer_, parseBuffer_,
                                           [](std::string &data) {
                                               return HealthResponse(data);
                                           });
}


Expected<HttpResponse<ExploreResponse>> HttpClient::explore(const Area &area) noexcept {
    marshalArea(area, postDataBuffer_);
    Measure<std::chrono::microseconds> tm;
    auto ret = makeRequest(exploreURL_, postDataBuffer_.c_str());
    if (ret.hasError()) {
        return ret.error();
    }
    auto latency = tm.getDuration();

    stats_->recordEndpointStats("explore", ret.get(), latency.count());
    stats_->recordExploreRequest((int64_t) area.getArea());
    return prepareResponse<ExploreResponse>(ret, resp_.data, latency, valueBuffer_, parseBuffer_,
                                            [this](std::string &data) {
                                                return unmarshalExploreResponse(data, this->valueBuffer_,
                                                                                this->parseBuffer_);
                                            });
}

Expected<HttpResponse<Wallet>> HttpClient::cash(const TreasureID &treasureId) noexcept {
    marshalTreasureId(treasureId, postDataBuffer_);
    Measure<std::chrono::microseconds> tm;
    auto ret = makeRequest(cashURL_, postDataBuffer_.c_str());
    if (ret.hasError()) {
        return ret.error();
    }
    auto latency = tm.getDuration();

    stats_->recordEndpointStats("cash", ret.get(), latency.count());
    return prepareResponse<Wallet>(ret, resp_.data, latency, valueBuffer_, parseBuffer_, [this](std::string &data) {
        Wallet w;
        unmarshallWallet(data, this->valueBuffer_, this->parseBuffer_, w);
        return w;
    });
}

Expected<HttpResponse<std::vector<TreasureID>>> HttpClient::dig(DigRequest request) noexcept {
    marshalDig(request.licenseId_, request.posX_, request.posY_, request.depth_, postDataBuffer_);
    Measure<std::chrono::microseconds> tm;
    auto ret = makeRequest(digURL_, postDataBuffer_.c_str());
    if (ret.hasError()) {
        return ret.error();
    }
    auto latency = tm.getDuration();

    stats_->recordEndpointStats("dig", ret.get(), latency.count());
    return prepareResponse<std::vector<TreasureID>>(ret, resp_.data, latency, valueBuffer_, parseBuffer_,
                                                    [this](std::string &data) {
                                                        std::vector<TreasureID> buf;
                                                        unmarshalTreasuriesList(data, this->valueBuffer_,
                                                                                this->parseBuffer_, buf);
                                                        return buf;
                                                    });
}

Expected<HttpResponse<License>> HttpClient::issueFreeLicense() noexcept {
    marshalFreeIssueLicenseRequest(postDataBuffer_);
    Measure<std::chrono::microseconds> tm;
    auto ret = makeRequest(issueLicenseURL_, postDataBuffer_.c_str());
    if (ret.hasError()) {
        return ret.error();
    }
    auto latency = tm.getDuration();

    stats_->recordEndpointStats("issue_license_free", ret.get(), latency.count());
    return prepareResponse<License>(ret, resp_.data, latency, valueBuffer_, parseBuffer_, [this](std::string &data) {
        return unmarshalLicense(data, this->valueBuffer_, this->parseBuffer_);
    });
}

Expected<HttpResponse<License>> HttpClient::issueLicense(CoinID coinId) noexcept {
    marshalIssueLicenseRequest(coinId, postDataBuffer_);
    Measure<std::chrono::microseconds> tm;
    auto ret = makeRequest(issueLicenseURL_, postDataBuffer_.c_str());
    if (ret.hasError()) {
        return ret.error();
    }
    auto latency = tm.getDuration();

    stats_->recordEndpointStats("issue_license_paid", ret.get(), latency.count());
    return prepareResponse<License>(ret, resp_.data, latency, valueBuffer_, parseBuffer_, [this](std::string &data) {
        return unmarshalLicense(data, this->valueBuffer_, this->parseBuffer_);
    });
}

Expected<int32_t>
HttpClient::makeRequest(const std::string &url, const char *data) noexcept {
    if (curl_easy_setopt(session_, CURLOPT_URL, url.c_str()) != CURLE_OK) {
        stats_->incCurlErrCnt();
        return ErrorCode::kErrCurl;
    }
    this->resp_.data.clear();

    if (data != nullptr) {
        if (curl_easy_setopt(session_, CURLOPT_POSTFIELDS, data) != CURLE_OK) {
            stats_->incCurlErrCnt();
            return ErrorCode::kErrCurl;
        }
    } else {
        if (curl_easy_setopt(session_, CURLOPT_HTTPGET, 1L) != CURLE_OK) {
            stats_->incCurlErrCnt();
            return ErrorCode::kErrCurl;
        }
    }

//    debugf("making request: %s data: %s", url.c_str(), data);
    auto reqResult = curl_easy_perform(session_);
    stats_->incRequestsCnt();
    if (reqResult != CURLE_OK) {
//        errorf("curl_easy_perform failed: %s err code: %d err message: %s errbuf: %s", url.c_str(), reqResult,
//               curl_easy_strerror(reqResult),
//               errbuf_);
        stats_->incCurlErrCnt();
        if (reqResult == CURLE_OPERATION_TIMEDOUT) {
            return ErrorCode::kErrCurlTimeout;
        } else {
            return ErrorCode::kErrCurl;
        }
    }

    long code;
    if (curl_easy_getinfo(session_, CURLINFO_RESPONSE_CODE, &code) != CURLE_OK) {
        stats_->incCurlErrCnt();
        return ErrorCode::kErrCurl;
    }
    return (int32_t) code;
}

