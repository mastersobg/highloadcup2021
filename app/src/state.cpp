#include "state.h"
#include "app.h"

ExploreAreaPtr State::fetchNextExploreArea() noexcept {
#ifdef _HLC_DEBUG
    assert(!exploreQueue_.empty());
    int cnt{0};
    for (const auto &v:exploreQueue_) {
        if (v->getLeftTreasuriesCnt() == 0) {
            ++cnt;
        }
    }
    getApp().getStats().setQueueSize((int64_t) exploreQueue_.size());
    getApp().getStats().setQueueSize(cnt);
#endif
    auto ret = *std::min_element(exploreQueue_.begin(), exploreQueue_.end());
    auto child = ret->getChildForRequest();
    if (child) {
        return child;
    }
    size_t pos{1};
    for (auto it = exploreQueue_.begin() + 1; it < exploreQueue_.end(); it++, pos++) {
        std::nth_element(exploreQueue_.begin(), it, exploreQueue_.end());
        auto c = exploreQueue_[pos]->getChildForRequest();
        if (c) {
            return c;
        }
    }
#ifdef _HLC_DEBUG
    assert(false);
#endif
    return nullptr;
}
