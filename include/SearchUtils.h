#ifndef YANDEX_DISK_CPP_CLIENT_SEARCHUTILS_H
#define YANDEX_DISK_CPP_CLIENT_SEARCHUTILS_H
#pragma once

#include <nlohmann/json.hpp>

namespace search_utils {
    std::vector<std::string> findPathsByName(
        const std::string& name,
        const std::string& start_path,
        std::function<nlohmann::json(const std::string&)> list_func,
        bool recursive = true);
}

#endif //YANDEX_DISK_CPP_CLIENT_SEARCHUTILS_H
