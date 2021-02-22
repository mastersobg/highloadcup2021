#include <iostream>
#include <csignal>
#include <vector>
#include "log.h"
#include <rapidjson/document.h>
#include <curl/curl.h>
#include <cstdlib>
#include <string>
#include "http_client.h"

void printBuildType() {
#ifndef BUILD_TYPE
#define BUILD_TYPE "unknown"
#endif
    infof("Build type: %s", BUILD_TYPE);
}

int main() {
    printBuildType();
    if (auto val = curl_global_init(CURL_GLOBAL_ALL)) {
        errorf("curl global init failed: %d", val);
        return 0;
    }

    std::signal(SIGINT, []([[maybe_unused]]int signal) {
        exit(0);
    });

    std::string address;
    if (auto a = std::getenv("ADDRESS")) {
        address = a;
    }
    if (address.empty()) {
        errorf("empty ADDRESS");
        return 0;
    }
    std::string port = "8000";//std::getenv("Port");
    std::string schema = "http";// std::getenv("Schema");

    HttpClient client(address, port, schema);

    for (;;) {
        auto ret = client.checkHealth();
        if (ret.hasError()) {
            continue;
        }

        auto resp = ret.get();
        if (resp.hasError()) {
            continue;
        }

        if (resp.getHttpCode() == 200) {
            break;
        }
    }

    int32_t digLeft{0};
    LicenseID licenseId;
    std::vector<TreasureID> treasuries;
    for (auto i = 0; i < 3500; i++) {
        for (auto j = 0; j < 3500; j++) {
            auto ret = client.explore(Area((int16_t) i, (int16_t) j, 1, 1));
            if (ret.hasError()) {
                errorf("error: %d", ret.error());
                return 0;
            }

            auto resp = std::move(ret).get();

            if (resp.hasError()) {
                auto httpCode = resp.getHttpCode();
                auto errResp = std::move(resp).getErrResponse();
                errorf("explore: http code: %d error code: %d message: %s", httpCode, errResp.errorCode_,
                       errResp.message_.c_str());
                continue;
            }

            auto exploreResp = std::move(resp).getResponse();
            if (exploreResp.amount_ <= 0) {
                continue;
            }

            int treasuriesLeft = (int) exploreResp.amount_;
            debugf("%d %d: %d", i, j, treasuriesLeft);
            int depth{1};
            int treasuriesGathered{0};
            while (treasuriesLeft > 0) {
                if (digLeft <= 0) {
                    auto issueLicenseRet = client.issueLicense(Wallet());
                    if (issueLicenseRet.hasError()) {
                        errorf("error: %d", issueLicenseRet.error());
                        continue;
                    }

                    auto issueLicenseResp = issueLicenseRet.get();
                    if (issueLicenseResp.hasError()) {
//                        auto httpCode = issueLicenseResp.getHttpCode();
                        auto errResp = std::move(issueLicenseResp).getErrResponse();
//                        errorf("issue license: http code: %d error code: %d message: %s", httpCode,
//                               errResp.errorCode_, errResp.message_.c_str());
                        continue;
                    }

                    auto issueLicense = std::move(issueLicenseResp).getResponse();
                    licenseId = issueLicense.id_;
                    digLeft = (int32_t) issueLicense.digAllowed_;
                }

                treasuries.clear();
                auto digRet = client.dig(licenseId, (int16_t) i, (int16_t) j, (int8_t) depth, treasuries);
                if (digRet.hasError()) {
                    errorf("error: %d", digRet.error());
                    continue;
                }

                auto digResp = digRet.get();
                if (digResp.hasError()) {
                    auto httpCode = digResp.getHttpCode();
                    auto errResp = std::move(digResp).getErrResponse();
                    if (httpCode != 404) {
                        errorf("dig: http code: %d error code: %d message: %s", httpCode,
                               errResp.errorCode_, errResp.message_.c_str());
                    }
                }
                if (!treasuries.empty()) {
                    debugf("treasuries found: %d depth: %d", treasuries.size(), depth);
                }

                digLeft--;
                depth++;

                treasuriesLeft -= (int) treasuries.size();
                treasuriesGathered += (int) treasuries.size();
                for (const auto &id : treasuries) {
                    Wallet w;
                    auto cashRet = client.cash(id, w);
                    if (cashRet.hasError()) {
                        errorf("error: %d", cashRet.error());
                    }
                }
            }
        }
    }

    curl_global_cleanup();
    return 0;
}
