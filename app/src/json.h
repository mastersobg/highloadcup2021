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

#endif //HIGHLOADCUP2021_JSON_H
