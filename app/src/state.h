#ifndef HIGHLOADCUP2021_STATE_H
#define HIGHLOADCUP2021_STATE_H

#include <cstdint>
#include <utility>
#include "const.h"
#include <array>
#include "api_entities.h"
#include "error.h"
#include "log.h"
#include <list>
#include <stdexcept>

class State {
private:
    int16_t lastX_{0};
    int16_t lastY_{0};

    std::array<License, kMaxLicensesCount> licenses_{};
    std::array<std::array<int32_t, kFieldMaxX>, kFieldMaxY> leftTreasuriesAmount_{};
    std::list<CoinID> coins_;

public:
    State() = default;

    State(const State &s) = delete;

    State(State &&s) = delete;

    State &operator=(const State &s) = delete;

    State &operator=(State &&s) = delete;

    int16_t &lastX() noexcept {
        return lastX_;
    }

    int16_t &lastY() noexcept {
        return lastY_;
    }

    std::pair<int16_t, int16_t> nextExploreCoord() {
        auto x = lastX_;
        auto y = (int16_t) (lastY_ + 1);
        if (y == kFieldMaxY) {
            x++;
            y = 0;
        }

        lastX_ = x;
        lastY_ = y;
        return {x, y};
    }

    void addLicence(License l) {
        for (auto &v: licenses_) {
            if (v.digAllowed_ == v.digConfirmed_) {
                v = l;
                break;
            }
        }
    }

    Expected<License> getAvailableLicenceID() {
        for (auto &v : licenses_) {
            if (v.digAllowed_ - v.digUsed_ > 0) {
                v.digUsed_++;
                return v;
            }
        }
        return ErrorCode::kNoAvailableLicense;
    }

    void setLeftTreasuriesAmount(int16_t x, int16_t y, int32_t amount) {
        leftTreasuriesAmount_[(size_t) x][(size_t) y] = amount;
    }

    int32_t getLeftTreasuriesAmount(int16_t x, int16_t y) {
        return leftTreasuriesAmount_[(size_t) x][(size_t) y];
    }

    void addCoins(const Wallet &w) {
        for (const auto &v : w.coins) {
            coins_.push_back(v);
        }
    }

    bool hasCoins() {
        return !coins_.empty();
    }

    CoinID borrowCoin() {
        auto ret = coins_.front();
        coins_.pop_front();
        return ret;
    }

    License &getLicenseById(LicenseID id) {
        for (auto &v : licenses_) {
            if (v.id_ == id) {
                return v;
            }
        }

        throw std::runtime_error("license not found");
    }

    int getInUseLicensesCount() noexcept {
        int cnt = 0;
        for (const auto &v: licenses_) {
            if (v.digAllowed_ > v.digConfirmed_) {
                cnt++;
            }
        }
        return cnt;
    }

    void printLicenses() noexcept {
        for (const auto &v: licenses_) {
            errorf("id: %d allowed: %d used: %d confirmed: %d",
                   v.id_, v.digAllowed_, v.digUsed_, v.digConfirmed_);
        }
        debugf("==============");
    }

};


#endif //HIGHLOADCUP2021_STATE_H
