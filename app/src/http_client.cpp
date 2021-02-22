#include "http_client.h"
#include "log.h"
#include <stdexcept>
#include <curl/curl.h>
#include "json.h"

size_t httpCallback(char *ptr, size_t size, size_t nmemb, void *ud) noexcept {
    auto resp = (respHolder *) ud;
    for (size_t i = 0; i < nmemb; i++) {
        resp->data += ptr[i];
    }
    return size * nmemb;
}

template<class T, class Convertor>
Expected<HttpResponse<T>> prepareResponse(Expected<int32_t> &response, std::string &data, Convertor convert) {
    auto code = response.get();
    if (code == 200) {
        return HttpResponse<T>(convert(data), code);
    } else {
        return HttpResponse<T>(unmarshalApiError(data), code);
    }
}

HttpClient::HttpClient(const std::string &address, const std::string &port,
                       const std::string &schema
) : errbuf_{0,},
    baseURL_{schema + "://" + address + ":" + port},
    checkHealthURL_{baseURL_ + "/health-check"},
    exploreURL_{baseURL_ + "/explore"} {

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

    curl_easy_setopt(session_, CURLOPT_VERBOSE, 1L);
}

HttpClient::~HttpClient() {
    curl_easy_cleanup(session_);
    curl_slist_free_all(headers_);
}

Expected<HttpResponse<HealthResponse>> HttpClient::checkHealth() noexcept {
    auto ret = makeRequest(checkHealthURL_, nullptr);
    if (ret.hasError()) {
        return ret.error();
    }

    return prepareResponse<HealthResponse>(ret, resp_.data, [](std::string &data) {
        return HealthResponse(data);
    });
}


Expected<HttpResponse<ExploreResponse>> HttpClient::explore(const Area &area) noexcept {
    marshalArea(area, postDataBuffer_);
    auto ret = makeRequest(exploreURL_, postDataBuffer_.c_str());
    if (ret.hasError()) {
        return ret.error();
    }

    return prepareResponse<ExploreResponse>(ret, resp_.data, [](std::string &data) {
        return unmarshalExploreResponse(data);
    });
}

Expected<int32_t>
HttpClient::makeRequest(const std::string &url, const char *data) noexcept {
    if (curl_easy_setopt(session_, CURLOPT_URL, url.c_str()) != CURLE_OK) {
        return ErrorCode::kErrCurl;
    }
    this->resp_.data.clear();

    if (data != nullptr) {
        if (curl_easy_setopt(session_, CURLOPT_POSTFIELDS, data) != CURLE_OK) {
            return ErrorCode::kErrCurl;
        }
    } else {
        if (curl_easy_setopt(session_, CURLOPT_HTTPGET, 1L) != CURLE_OK) {
            return ErrorCode::kErrCurl;
        }
    }

    auto reqResult = curl_easy_perform(session_);
    if (reqResult != CURLE_OK) {
        errorf("curl_easy_perform failed: %s %s %s", checkHealthURL_.c_str(), curl_easy_strerror(reqResult), errbuf_);
        return ErrorCode::kErrCurl;
    }

    long code;
    if (curl_easy_getinfo(session_, CURLINFO_RESPONSE_CODE, &code) != CURLE_OK) {
        return ErrorCode::kErrCurl;
    }
    return (int32_t) code;
}


