#include <iostream>
#include <csignal>
#include <vector>
#include "log.h"
#include <rapidjson/document.h>
#include <curl/curl.h>
#include <cstdlib>
#include <string>
#include "http_client.h"

int main() {
    if (auto val = curl_global_init(CURL_GLOBAL_ALL)) {
        errorf("curl global init failed: %d", val);
        return 0;
    }

    std::signal(SIGINT, []([[maybe_unused]]int signal) {
        exit(0);
    });

    std::string address = std::getenv("ADDRESS");
    std::string port = std::getenv("Port");
    std::string schema = std::getenv("Schema");

    HttpClient client(address, port, schema);
    auto res = client.explore(Area(0, 0, 3500, 3500));
    if (res.hasError()) {
        errorf("error: %d", res.error());
        return 0;
    }
    auto resp = std::move(res).get();
    if (resp.hasError()) {
        auto errResp = std::move(resp).getErrResponse();
        errorf("errCode: %d errMessage: %s", errResp.errorCode_, errResp.message_.c_str());
    } else {
        auto successResp = std::move(resp).getResponse();
        debugf("success amount: %d", successResp.amount_);
    }

    auto healthRes = client.checkHealth();

    curl_global_cleanup();
    return 0;
}
