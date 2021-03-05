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
    int16_t posX_;
    int16_t posY_;
    int16_t sizeX_;
    int16_t sizeY_;

    Area() = default;

    Area(int16_t posX, int16_t posY, int16_t sizeX, int16_t sizeY) : posX_{posX}, posY_{posY}, sizeX_{sizeX},
                                                                     sizeY_{sizeY} {}
};

struct ExploreResponse {
    Area area_;
    uint32_t amount_;

    ExploreResponse(Area area, uint32_t amount) : area_{area}, amount_{amount} {}
};

using CoinID = std::uint32_t;

struct Wallet {
    std::vector<CoinID> coins;

//    Wallet() = default;
//
//    Wallet(const Wallet &o) = delete;
//
//    Wallet(Wallet &&o) = default;
//
//    Wallet &operator=(const Wallet &o) = delete;
//
//    Wallet &operator=(Wallet &&o) = default;
//
};

using TreasureID = std::string;
using LicenseID = int32_t;

struct License {
    int32_t id_;
    uint32_t digAllowed_;
    uint32_t digUsed_;
    uint32_t digConfirmed_;

    License() : id_{0}, digAllowed_{0}, digUsed_{0}, digConfirmed_{0} {}

    License(int32_t id, uint32_t digAllowed, uint32_t digUsed) :
            id_{id},
            digAllowed_{digAllowed},
            digUsed_{digUsed},
            digConfirmed_{0} {}
};

struct DigRequest {
    LicenseID licenseId_;
    int16_t posX_, posY_;
    int8_t depth_;

    DigRequest(LicenseID licenseID, int16_t posX, int16_t posY, int8_t depth) :
            licenseId_{licenseID},
            posX_{posX},
            posY_{posY},
            depth_{depth} {}

};

#endif //HIGHLOADCUP2021_API_ENTITIES_H
