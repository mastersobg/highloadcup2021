#ifndef HIGHLOADCUP2021_JSON_H
#define HIGHLOADCUP2021_JSON_H

#include "api_entities.h"
#include <string>
#include "const.h"

ApiError unmarshalApiError(std::string &data, JsonBufferType *valueBuffer, JsonBufferType *parseBuffer) noexcept;

ExploreResponse
unmarshalExploreResponse(std::string &data, JsonBufferType *valueBuffer, JsonBufferType *parseBuffer) noexcept;

void marshalArea(const Area &area, std::string &buffer) noexcept;

void
unmarshallWallet(std::string &data, JsonBufferType *valueBuffer, JsonBufferType *parseBuffer, Wallet &buf) noexcept;

void marshalTreasureId(const std::string &treasureId, std::string &buffer) noexcept;

void marshalDig(LicenseID licenseId, int16_t posX, int16_t posY, int8_t depth, std::string &buffer) noexcept;

void unmarshalTreasuriesList(std::string &data, JsonBufferType *valueBuffer, JsonBufferType *parseBuffer,
                             std::vector<TreasureID> &buf) noexcept;

void marshalIssueLicenseRequest(const std::vector<CoinID> &coinIds, std::string &buffer) noexcept;

void marshalFreeIssueLicenseRequest(std::string &buffer) noexcept;

License unmarshalLicense(std::string &data, JsonBufferType *valueBuffer, JsonBufferType *parseBuffer) noexcept;

#endif //HIGHLOADCUP2021_JSON_H
