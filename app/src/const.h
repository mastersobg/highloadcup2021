#ifndef HIGHLOADCUP2021_CONST_H
#define HIGHLOADCUP2021_CONST_H

#include <cstdlib>

using JsonBufferType = char;

constexpr size_t kJsonValueBufferCap = 1 << 15;
constexpr size_t kJsonParseBufferCap = 1 << 15;
constexpr size_t kJsonBufferTypeSize = sizeof(JsonBufferType);
constexpr size_t kJsonValueBufferSize = kJsonValueBufferCap * kJsonBufferTypeSize;
constexpr size_t kJsonParseBufferSize = kJsonParseBufferCap * kJsonBufferTypeSize;

constexpr size_t kMaxApiRequestsQueueSize = 10'000'000;

constexpr size_t kApiThreadCount = 100;
constexpr int64_t kMaxRPS = 100'000'000;

constexpr size_t kFieldMaxX = 3'500;
constexpr size_t kFieldMaxY = 3'500;

constexpr size_t kMaxLicensesCount = 10;


#endif //HIGHLOADCUP2021_CONST_H
