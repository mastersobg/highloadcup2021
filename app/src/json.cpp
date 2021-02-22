#include "json.h"
#include "log.h"
#include <rapidjson/document.h>
#include "util.h"
#include "const.h"

rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<>, rapidjson::MemoryPoolAllocator<>>
parse(std::string &data, JsonBufferType *valueBuffer, JsonBufferType *parseBuffer) {
    rapidjson::MemoryPoolAllocator<> valueAllocator(valueBuffer, kJsonValueBufferSize);
    rapidjson::MemoryPoolAllocator<> parseAllocator(parseBuffer, kJsonParseBufferSize);
    rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<>, rapidjson::MemoryPoolAllocator<>> d(
            &valueAllocator, 0, &parseAllocator);
    d.ParseInsitu(data.data());
    debugf("value allocator: %d parse allocator: %d", valueAllocator.Size(), parseAllocator.Size());
    return d;
}

Area unmarshalArea(
        const rapidjson::GenericValue<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>> &json) noexcept {
    auto posX = json["posX"].GetInt();
    auto posY = json["posY"].GetInt();
    auto sizeX = json["sizeX"].GetInt();
    auto sizeY = json["sizeY"].GetInt();
    return Area((int16_t) posX, (int16_t) posY, (int16_t) sizeX, (int16_t) sizeY);
}

ApiError unmarshalApiError(std::string &data, JsonBufferType *valueBuffer, JsonBufferType *parseBuffer) noexcept {
    auto d = parse(data, valueBuffer, parseBuffer);
    if (d.IsObject() && d.HasMember("code") && d.HasMember("message")) {
        return ApiError(d["code"].GetInt(), d["message"].GetString());
    } else {
        return ApiError(ApiErrorCodeUnknown, "");
    }
}

ExploreResponse
unmarshalExploreResponse(std::string &data, JsonBufferType *valueBuffer, JsonBufferType *parseBuffer) noexcept {
    auto d = parse(data, valueBuffer, parseBuffer);
    auto area = unmarshalArea(d["area"].GetObject());
    auto amount = d["amount"].GetInt();
    return ExploreResponse(area, (uint32_t) amount);
}

void
unmarshallWallet(std::string &data, JsonBufferType *valueBuffer, JsonBufferType *parseBuffer, Wallet &buf) noexcept {
    buf.coins.clear();

    auto d = parse(data, valueBuffer, parseBuffer);
    for (auto &val : d.GetArray()) {
        buf.coins.push_back((uint32_t) val.GetInt());
    }
}

void marshalTreasureId(const TreasureID &treasureId, std::string &buffer) noexcept {
    buffer.clear();

    buffer += '"';
    buffer += treasureId;
    buffer += '"';
}

void marshalArea(const Area &area, std::string &buffer) noexcept {
    buffer.clear();

    buffer += "{";
    buffer += "\"posX\":";
    writeIntToString(area.posX_, buffer);
    buffer += ',';
    buffer += "\"posY\":";
    writeIntToString(area.posY_, buffer);
    buffer += ',';
    buffer += "\"sizeX\":";
    writeIntToString(area.sizeX_, buffer);
    buffer += ",";
    buffer += "\"sizeY\":";
    writeIntToString(area.sizeY_, buffer);
    buffer += "}";
}