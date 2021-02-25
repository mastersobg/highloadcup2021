#include <iostream>
#include <csignal>
#include <vector>
#include "log.h"
#include <curl/curl.h>
#include <cstdlib>
#include <string>
#include "http_client.h"
#include "app.h"
#include "util.h"
#include <thread>
#include <random>
#include <set>

void printBuildInfo() {
#ifndef BUILD_TYPE
#define BUILD_TYPE "unknown"
#endif

#ifndef COMMIT_HASH
#define COMMIT_HASH "unknown"
#endif

    infof("Build type: %s commit hash: %s", BUILD_TYPE, COMMIT_HASH);
}

int runExplore() {
    std::string address = std::getenv("ADDRESS");
    std::string port = "8000";
    std::string schema = "http";
    infof("address: %s port: %s schema: %s", address.c_str(), port.c_str(), schema.c_str());

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

    std::random_device randomDevice;
    std::default_random_engine rnd{randomDevice()};
    std::uniform_int_distribution distribution{0, 3500 - 1};

//    std::array<std::array<uint32_t, 10>, 10> arr{};
    int emptyCnt{0};
    int totalCnt{0};
    for (size_t i = 0; i < 3500; i += 5) {
        for (size_t j = 0; j < 3500; j += 5) {
            ++totalCnt;
            auto ret = client.explore(Area((int16_t) i, (int16_t) j, (int16_t) 5, (int16_t) 5));
            if (ret.hasError()) {
                errorf("error: %d", ret.error());
                goto endloop;
            }

            auto resp = std::move(ret).get();

            if (resp.hasError()) {
                auto httpCode = resp.getHttpCode();
                auto errResp = std::move(resp).getErrResponse();
                errorf("explore: http code: %d error code: %d message: %s", httpCode, errResp.errorCode_,
                       errResp.message_.c_str());
                errorf("%d %d %d %d", i, j, 350, 350);
                continue;
            }

            auto respVal = std::move(resp).getResponse();
            if (respVal.amount_ == 0) {
                emptyCnt++;
            }
        }
    }

    endloop:

    debugf("Got %d empty squares out of %d", emptyCnt, totalCnt);
//    std::string logStr{};
//    for (const auto &a:arr) {
//        for (const auto &v:a) {
//            writeIntToString(v, logStr);
//            logStr += " ";
//        }
//        logStr += "\n";
//    }
//    debugf("\n%s", logStr.c_str());
//    for (auto it = 0; it < (1 << 30); it++) {
//        auto x1 = distribution(rnd);
//        auto y1 = distribution(rnd);
////        auto s1 = distribution(rnd);
////        auto s2 = distribution(rnd);
//        auto s1 = 1;
//        auto s2 = 1;
//        auto x2 = x1 + s1;
//        auto y2 = y1 + s2;
//
//        if (s1 > 0 && s2 > 0 && x2 < 3500 && y2 < 3500) {
//
//        } else {
//            continue;
//        }
//        Measure<std::chrono::milliseconds> tm{};
//        auto ret = client.explore(Area((int16_t) x1, (int16_t) y1, (int16_t) s1, (int16_t) s2));
//        if (ret.hasError()) {
//            errorf("error: %d", ret.error());
//            return 0;
//        }
//
//        auto resp = std::move(ret).get();
//
//        if (resp.hasError()) {
//            auto httpCode = resp.getHttpCode();
//            auto errResp = std::move(resp).getErrResponse();
//            errorf("explore: http code: %d error code: %d message: %s", httpCode, errResp.errorCode_,
//                   errResp.message_.c_str());
//            errorf("%d %d %d %d", x1, y1, x2, y2);
//            continue;
//        }
//
//        getApp().getStats().recordExploreArea(s1 * s2, tm.getInt32());
//    }
    return 0;
}

int run() {
    std::string address = std::getenv("ADDRESS");
    std::string port = "8000";
    std::string schema = "http";
    infof("address: %s port: %s schema: %s", address.c_str(), port.c_str(), schema.c_str());

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

    std::random_device randomDevice;
    std::default_random_engine rnd{randomDevice()};
    std::uniform_int_distribution distribution{0, 3500 - 1};
    int32_t digLeft{0};
    LicenseID licenseId;
    std::vector<TreasureID> treasuries;
    Wallet wallet{};
    std::set<std::pair<int, int>> usedCells{};
    for (auto x = 0; x < (1 << 30); x++) {
        auto i = distribution(rnd);
        auto j = distribution(rnd);
        auto p = std::make_pair(i, j);
        if (usedCells.find(p) != usedCells.end()) {
            continue;
        }
        usedCells.insert(p);
//        debugf("i: %d j: %d", i, j);
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
        getApp().getStats().recordExploreCell(exploreResp.amount_);
        if (exploreResp.amount_ <= 0) {
            continue;
        }

        int treasuriesLeft = (int) exploreResp.amount_;
        //debugf("%d %d: %d", i, j, treasuriesLeft);
        int depth{1};
        int treasuriesGathered{0};
        while (treasuriesLeft > 0) {
            if (digLeft <= 0) {
                Wallet w{};
                if (!wallet.coins.empty()) {
                    uint32_t c = wallet.coins[wallet.coins.size() - 1];
                    wallet.coins.pop_back();
                    w.coins.push_back(c);
                }
                auto issueLicenseRet = client.issueLicense(w);
                if (issueLicenseRet.hasError()) {
                    errorf("error: %d", issueLicenseRet.error());
                    return 0;
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
                return 0;
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
//                if (!treasuries.empty()) {
//                    debugf("treasuries found: %d depth: %d", treasuries.size(), depth);
//                }

            if (!treasuries.empty()) {
                getApp().getStats().recordTreasuireDepth(depth, (int) treasuries.size());
            }


            treasuriesLeft -= (int) treasuries.size();
            treasuriesGathered += (int) treasuries.size();
            for (const auto &id : treasuries) {
                Wallet w;
                auto cashRet = client.cash(id, w);
                if (cashRet.hasError()) {
                    errorf("error: %d", cashRet.error());
                    return 0;
                }

                if (cashRet.get().getHttpCode() == 200) {
                    getApp().getStats().recordCoinsDepth(depth, (int) w.coins.size());
                    for (const auto val : w.coins) {
                        wallet.coins.push_back(val);
                    }
                }
            }

            digLeft--;
            depth++;
        }
    }

    return 0;
}

int main() {
    printBuildInfo();

    std::thread statsThread(statsPrintLoop);

    if (auto val = curl_global_init(CURL_GLOBAL_ALL)) {
        errorf("curl global init failed: %d", val);
        return 0;
    }

    std::signal(SIGINT, []([[maybe_unused]]int signal) {
        exit(0);
    });

    std::vector<std::thread> threads{};
    for (auto i = 0; i < 1; i++) {
        std::thread t(runExplore);
        threads.push_back(std::move(t));
    }
    for (auto &t: threads) {
        t.join();
    }

    getApp().getStats().stop();
    statsThread.join();

    curl_global_cleanup();
    return 0;
}
