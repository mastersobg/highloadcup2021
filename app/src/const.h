#ifndef HIGHLOADCUP2021_CONST_H
#define HIGHLOADCUP2021_CONST_H

#include <cstdlib>
#include <array>

using JsonBufferType = char;

constexpr size_t kJsonValueBufferCap = 1 << 15;
constexpr size_t kJsonParseBufferCap = 1 << 15;
constexpr size_t kJsonBufferTypeSize = sizeof(JsonBufferType);
constexpr size_t kJsonValueBufferSize = kJsonValueBufferCap * kJsonBufferTypeSize;
constexpr size_t kJsonParseBufferSize = kJsonParseBufferCap * kJsonBufferTypeSize;

constexpr size_t kMaxApiRequestsQueueSize = 10'000'000;

constexpr size_t kApiThreadCount = 13;
constexpr int64_t kMaxRPS = 1'000'000;

constexpr size_t kFieldMaxX = 3'500;
constexpr size_t kFieldMaxY = 3'500;

struct ExploreAreaShift {
    int16_t height;
    int16_t width;
};
constexpr std::array<ExploreAreaShift, 5> kExploreAreas = {
        {
                {500, 10},
                {500, 1},
                {50, 1},
                {5, 1},
                {1, 1}
        }
};

constexpr size_t kTreasuriesCount = 490'000;

constexpr long kRequestTimeout = 1'000'000;

constexpr size_t kMaxLicensesCount = 10;

constexpr size_t kExploreConcurrentRequestsCnt{1};

constexpr int minDepthToCash{1};

constexpr int kLicensePrice{1};

constexpr int kMinCashRequestsToStartCashing{2500};

#endif //HIGHLOADCUP2021_CONST_H
