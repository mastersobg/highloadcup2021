#ifndef HIGHLOADCUP2021_API_ENTITIES_H
#define HIGHLOADCUP2021_API_ENTITIES_H

#include <cstdint>
#include <string>
#include <vector>

constexpr int32_t ApiErrorCodeUnknown = 1;

struct ApiError {
    int32_t errorCode_;
    std::string message_;

    ApiError(int32_t errorCode, std::string message) : errorCode_{errorCode},
                                                       message_{std::move(message)} {}
};

struct HealthResponse {
    std::string details_;

    explicit HealthResponse(std::string details) : details_{std::move(details)} {
    }
};

struct Area {
    const int16_t posX_;
    const int16_t posY_;
    const int16_t sizeX_;
    const int16_t sizeY_;

    Area(int16_t posX, int16_t posY, int16_t sizeX, int16_t sizeY) : posX_{posX}, posY_{posY}, sizeX_{sizeX},
                                                                     sizeY_{sizeY} {}
};

struct ExploreRequest {
    const Area area_;

    explicit ExploreRequest(Area a) : area_{a} {}
};

struct ExploreResponse {
    const Area area_;
    const uint32_t amount_;

    ExploreResponse(Area area, uint32_t amount) : area_{area}, amount_{amount} {}
};

struct Wallet {
    std::vector<uint32_t> coins;

    Wallet() = default;

    Wallet(const Wallet &o) = delete;

    Wallet(Wallet &&o) = default;

    Wallet &operator=(const Wallet &o) = delete;

    Wallet &operator=(Wallet &&o) = default;

};

using TreasureID = std::string;
using LicenseID = int32_t;

struct License {
    const int32_t id_;
    const uint32_t digAllowed_;
    const uint32_t digUsed_;

    License(int32_t id, uint32_t digAllowed, uint32_t digUsed) : id_{id}, digAllowed_{digAllowed}, digUsed_{digUsed} {}
};

#endif //HIGHLOADCUP2021_API_ENTITIES_H
