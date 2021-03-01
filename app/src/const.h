#ifndef HIGHLOADCUP2021_CONST_H
#define HIGHLOADCUP2021_CONST_H

using JsonBufferType = char;

constexpr size_t kJsonValueBufferCap = 1 << 15;
constexpr size_t kJsonParseBufferCap = 1 << 15;
constexpr size_t kJsonBufferTypeSize = sizeof(JsonBufferType);
constexpr size_t kJsonValueBufferSize = kJsonValueBufferCap * kJsonBufferTypeSize;
constexpr size_t kJsonParseBufferSize = kJsonParseBufferCap * kJsonBufferTypeSize;

constexpr size_t kMaxApiRequestsQueueSize = 50'000;

#endif //HIGHLOADCUP2021_CONST_H
