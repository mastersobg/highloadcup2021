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
#include <set>
#include "util.h"
#include <cassert>

struct DelayedDigRequest {
    int16_t x_, y_;
    int8_t depth_;

    DelayedDigRequest(int16_t x, int16_t y, int8_t depth) :
            x_{x}, y_{y}, depth_{depth} {}
};


struct ExploreAreaCmp {
    bool operator()(const ExploreAreaPtr &l, const ExploreAreaPtr &r) const noexcept {
        if (l->expectedTreasuriesCnt_ < r->expectedTreasuriesCnt_) {
            return false;
        }
        if (l->expectedTreasuriesCnt_ > r->expectedTreasuriesCnt_) {
            return true;
        }

        const Area &la = l->area_;
        const Area &ra = r->area_;

        if (la.posX_ != ra.posX_) {
            return la.posX_ < ra.posX_;
        }
        if (la.posY_ != ra.posY_) {
            return la.posY_ < ra.posY_;
        }

        if (la.sizeX_ != ra.sizeX_) {
            return la.sizeX_ < ra.sizeX_;
        }

        return la.sizeY_ < ra.sizeY_;
    }
};

class State {
private:
    std::array<License, kMaxLicensesCount> licenses_{};
    std::array<std::array<int32_t, kFieldMaxX>, kFieldMaxY> leftTreasuriesAmount_{};
    std::list<CoinID> coins_;
    std::list<DelayedDigRequest> digRequests_;
    std::multiset<ExploreAreaPtr, ExploreAreaCmp> exploreQueue_{};
    ExploreAreaPtr root_{nullptr};

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
#ifdef _HLC_DEBUG
        assert(exploreQueue_.count(ea) == 0);
#endif
        exploreQueue_.insert(std::move(ea));
    }

    ExploreAreaPtr fetchNextExploreArea() noexcept {
        auto first = exploreQueue_.begin();
        auto result = *first;
        exploreQueue_.erase(first);
        return result;

    }

    void setExpectedTreasuriesCnt(const ExploreAreaPtr &ea, double value) noexcept {
#ifdef _HLC_DEBUG
        assert(exploreQueue_.count(ea) == 1);
#endif
        auto node = exploreQueue_.extract(ea);
        node.value()->expectedTreasuriesCnt_ = value;
        exploreQueue_.insert(std::move(node));
    }

    bool hasMoreExploreAreas() noexcept {
        return !exploreQueue_.empty();
    }

    void removeExploreAreaFromQueue(const ExploreAreaPtr &ea) noexcept {
#ifdef _HLC_DEBUG
        assert(exploreQueue_.count(ea) <= 1);
#endif
        exploreQueue_.erase(ea);
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
