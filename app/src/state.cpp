#include "state.h"
#include "app.h"

ExploreAreaPtr State::fetchNextExploreArea() noexcept {
#ifdef _HLC_DEBUG
    assert(!exploreQueue_.empty());
    ExploreAreaPtr prev = nullptr;
    for (const auto &val : exploreQueue_) {
        if (prev != nullptr) {
            assert(prev <= val);
        }
        prev = val;
    }
#endif
    for (const auto &val: exploreQueue_) {
        auto child = val->getChildForRequest();
        if (child) {
            return child;
        }
    }
#ifdef _HLC_DEBUG
    assert(false);
#endif
    return nullptr;
}
