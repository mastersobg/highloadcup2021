#ifndef HIGHLOADCUP2021_API_ENTITIES_H
#define HIGHLOADCUP2021_API_ENTITIES_H

#include <cstdint>
#include <string>

constexpr int32_t ApiErrorCodeUnknown = 1;

struct ApiError {
    const int32_t errorCode_;
    const std::string message_;

    ApiError(int32_t errorCode, std::string message) : errorCode_{errorCode},
                                                       message_{std::move(message)} {}
};

struct HealthResponse {
    const std::string details_;

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

struct ExploreResponse {
    const Area area_;
    const uint32_t amount_;

    ExploreResponse(Area area, uint32_t amount) : area_{area}, amount_{amount} {}
};


#endif //HIGHLOADCUP2021_API_ENTITIES_H
