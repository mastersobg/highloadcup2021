#include "app.h"
#include <string>
#include "const.h"
#include <vector>

int main(int argc, char *argv[]) {
    if (argc % 2 != 1) {
        errorf("wrong number of arguments");
        return 0;
    }
    std::vector<ExploreAreaShift> arr;
    for (int i = 1; i < argc; i += 2) {
        auto h = std::stoi(std::string(argv[i]));
        auto w = std::stoi(std::string(argv[i + 1]));
        arr.push_back({(int16_t) h, (int16_t) w});
    }
    injectKExploreAreas(arr);
    for (const auto &v: getKExploreAreas()) {
        debugf("h: %d w: %d", v.height, v.width);
    }
    getApp().run();
    return 0;
}
