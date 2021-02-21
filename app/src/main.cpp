#include <csignal>
#include <vector>
#include "log.h"
#include <rapidjson/document.h>
#include <curl/curl.h>
#include <cstdlib>
#include <string>

struct response {
    std::string resp;
};

size_t callback(char *ptr, size_t size, size_t nmemb, [[maybe_unused]]void *ud) {
    debugf("size: %d", size);
    debugf("nmemb: %d", nmemb);
    std::string data(ptr, 0, nmemb);
    auto resp = (response *) ud;
    resp->resp += data;
    return size * nmemb;
}

char errbuf[CURL_ERROR_SIZE];

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
    std::string url = schema + "://" + address + ":" + port;

    debugf("Started %d", 1);
    rapidjson::Document d;
    char s[] = "{\"a\":1}";
    d.ParseInsitu(s);
    auto val = d["a"].GetInt();
    debugf("value: %d", val);

    CURL *curl = curl_easy_init();
    if (curl == nullptr) {
        errorf("curl was not initialized");
    }
    debugf("Connecting to: %s", (url + "/health-check").c_str());
    response resp;
    curl_easy_setopt(curl, CURLOPT_URL, (url + "/health-check").c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &resp);
    auto res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        errorf("curl_easy_perform failed: %s", curl_easy_strerror(res));
        if (strlen(errbuf) > 0) {
            debugf("errbuf: %s", errbuf);
        }
    } else {
        debugf("response: %s", resp.resp.c_str());
    }

    curl_easy_cleanup(curl);

    curl_global_cleanup();
    return 0;
}
