#include "const.h"

#include <utility>

std::vector<ExploreAreaShift> kExploreAreas;

const std::vector<ExploreAreaShift> &getKExploreAreas() {
    return kExploreAreas;
}

void injectKExploreAreas(std::vector<ExploreAreaShift> v) {
    kExploreAreas = std::move(v);
}
