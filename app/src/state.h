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
#include <algorithm>
#include <memory>
#include <vector>
#include "util.h"
#include <cassert>
#include <set>
#include <random>

struct DelayedDigRequest {
    int16_t x_, y_;
    int8_t depth_;

    DelayedDigRequest(int16_t x, int16_t y, int8_t depth) :
            x_{x}, y_{y}, depth_{depth} {}
};

class State {
private:
    int16_t lastX_{0};
    int16_t lastY_{0};

    std::array<License, kMaxLicensesCount> licenses_{};
    std::array<std::array<int32_t, kFieldMaxX>, kFieldMaxY> leftTreasuriesAmount_{};
    std::list<CoinID> coins_;
    std::list<DelayedDigRequest> digRequests_;
    std::vector<ExploreAreaPtr> exploreQueue_{};
    ExploreAreaPtr root_{nullptr};

    std::random_device randomDevice_;
    std::default_random_engine rnd_{randomDevice_()};
    std::uniform_int_distribution<> distribution_{0, 3500 - 1};


    void removeFromExploreQueue(size_t pos) noexcept {
        exploreQueue_[pos] = std::move(exploreQueue_.back());
        exploreQueue_.pop_back();
    }

public:
    State() = default;

    State(const State &s) = delete;

    State(State &&s) = delete;

    State &operator=(const State &s) = delete;

    State &operator=(State &&s) = delete;

    ~State() {
        cleanExploreAreaPtrs(root_);
        root_ = nullptr;
    }

    void cleanExploreAreaPtrs(const ExploreAreaPtr &node) {
        for (const auto &child : node->children_) {
            cleanExploreAreaPtrs(child);
        }
        node->children_.resize(0);
        node->parent_ = nullptr;
    }


    void setRootExploreArea(ExploreAreaPtr r) noexcept {
        root_ = std::move(r);
    }

    void addExploreArea(ExploreAreaPtr ea) noexcept {
        exploreQueue_.push_back(std::move(ea));
    }

    ExploreAreaPtr fetchNextExploreArea() noexcept;

    bool hasMoreExploreAreas() noexcept {
        return !exploreQueue_.empty();
    }

    void removeExploreAreaFromQueue(const ExploreAreaPtr &ea) noexcept {
        for (size_t i = 0; i < exploreQueue_.size(); i++) {
            const auto &v = exploreQueue_[i];
            if (v == ea) {
                removeFromExploreQueue(i);
                break;
            }
        }
    }

    std::pair<int16_t, int16_t> nextExploreCoord() {
        auto x = distribution_(rnd_);
        auto y = distribution_(rnd_);
        return {x, y};
//        auto x = lastX_;
//        auto y = (int16_t) (lastY_ + 1);
//        if (y == kFieldMaxY) {
//            x++;
//            y = 0;
//        }
//
//        lastX_ = x;
//        lastY_ = y;
//        return {x, y};
    }
    void addLicence(License l) {
        for (auto &v: licenses_) {
            if (v.digAllowed_ == v.digConfirmed_) {
                v = l;
                break;
            }
        }
    }

    bool hasAvailableLicense() {
        return std::any_of(licenses_.begin(), licenses_.end(), [](const License &l) {
            return l.digAllowed_ - l.digUsed_ > 0;
        });
    }

    [[nodiscard]] Expected<LicenseID> reserveAvailableLicenseId() {
        for (auto &v : licenses_) {
            if (v.digAllowed_ - v.digUsed_ > 0) {
                v.digUsed_++;
                return v.id_;
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

    size_t getCoinsAmount() {
        return coins_.size();
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

    void addDigRequest(DelayedDigRequest r) noexcept {
        digRequests_.push_back(r);
    }

    bool hasQueuedDigRequests() noexcept {
        return !digRequests_.empty();
    }

    DelayedDigRequest getNextDigRequest() noexcept {
        auto r = digRequests_.front();
        digRequests_.pop_front();
        return r;
    }
};


#endif //HIGHLOADCUP2021_STATE_H
