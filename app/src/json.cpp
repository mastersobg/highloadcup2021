#include "json.h"
#include "log.h"
#include <rapidjson/document.h>
#include "util.h"

Area unmarshalArea(
        const rapidjson::GenericValue<rapidjson::UTF8<char>, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>> &json) noexcept {
    auto posX = json["posX"].GetInt();
    auto posY = json["posY"].GetInt();
    auto sizeX = json["sizeX"].GetInt();
    auto sizeY = json["sizeY"].GetInt();
    return Area((int16_t) posX, (int16_t) posY, (int16_t) sizeX, (int16_t) sizeY);
}

ApiError unmarshalApiError(std::string &data) noexcept {
    rapidjson::Document d;
    d.ParseInsitu(data.data());
    if (d.IsObject() && d.HasMember("code") && d.HasMember("message")) {
        return ApiError(d["code"].GetInt(), d["message"].GetString());
    } else {
        return ApiError(ApiErrorCodeUnknown, "");
    }
}

ExploreResponse unmarshalExploreResponse(std::string &data) noexcept {
    rapidjson::Document d;
    d.Parse(data.data());
    auto area = unmarshalArea(d["area"].GetObject());
    auto amount = d["amount"].GetInt();
    return ExploreResponse(area, (uint32_t) amount);
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