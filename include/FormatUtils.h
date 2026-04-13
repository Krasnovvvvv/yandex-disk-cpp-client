#ifndef YANDEX_DISK_CPP_CLIENT_FORMATUTILS_H
#define YANDEX_DISK_CPP_CLIENT_FORMATUTILS_H
#pragma once

#include <nlohmann/json.hpp>

namespace format_utils {
    std::string formatBytes(std::uint64_t bytes);

    std::string formatQuotaInfo(const nlohmann::json& quota);

    std::string formatResourceList(const nlohmann::json& json);

    std::string formatTrashResourceList(const nlohmann::json& json);

    std::string formatResourceInfo(const nlohmann::json& info);
}

#endif //YANDEX_DISK_CPP_CLIENT_FORMATUTILS_H