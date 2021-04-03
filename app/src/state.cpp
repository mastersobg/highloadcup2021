#include "state.h"
#include "app.h"

ExploreAreaPtr State::fetchNextExploreArea() noexcept {
    if (exploreQueue_.empty()) {
        return nullptr;
    }
#ifdef _HLC_DEBUG
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
    return nullptr;
}
