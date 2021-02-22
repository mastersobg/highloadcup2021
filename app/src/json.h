#ifndef HIGHLOADCUP2021_JSON_H
#define HIGHLOADCUP2021_JSON_H

#include "api_entities.h"
#include <string>

ApiError unmarshalApiError(std::string &data) noexcept;

ExploreResponse unmarshalExploreResponse(std::string &data) noexcept;

void marshalArea(const Area &area, std::string &buffer) noexcept;

#endif //HIGHLOADCUP2021_JSON_H
