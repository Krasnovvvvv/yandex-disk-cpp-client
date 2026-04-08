#include "FormatUtils.h"

std::string format_utils::formatBytes(std::uint64_t bytes) {
    std::ostringstream oss;
    auto value = static_cast<double>(bytes);
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int i = 0;
    while (value >= 1024 && i < 4) {
        value /= 1024;
        ++i;
    }
    oss << std::fixed << std::setprecision(2) << value << " " << units[i];
    return oss.str();
}

std::string format_utils::formatQuotaInfo(const nlohmann::json& quota) {
    std::ostringstream oss;
    oss << "Total space: " << formatBytes(quota["total_space"].get<uint64_t>()) << "\n";
    oss << "Used: " << formatBytes(quota["used_space"].get<uint64_t>()) << "\n";
    oss << "In trash: " << formatBytes(quota["trash_size"].get<uint64_t>()) << "\n";
    return oss.str();
}

std::string format_utils::formatResourceList(const nlohmann::json& json) {
    std::ostringstream oss;
    int idx = 1;
    for (const auto& item : json["_embedded"]["items"]) {
        oss << idx++ << ". " << item["name"].get<std::string>() << "\n";
        oss << "   Type: " << item["type"].get<std::string>() << "\n";
        oss << "   Path: " << item["path"].get<std::string>() << "\n";
        if (item.contains("public_url") && !item["public_url"].is_null())
            oss << "   Public URL: " << item["public_url"].get<std::string>() << "\n";
        else
            oss << "   Public URL: is missing\n";
        oss << "\n";
    }
    return oss.str();
}

std::string format_utils::formatTrashResourceList(const nlohmann::json& json) {
    std::ostringstream oss;
    int idx = 1;
    if (json.contains("_embedded") &&
    json["_embedded"].contains("items") &&
    !json["_embedded"]["items"].empty()) {
        for (const auto& item : json["_embedded"]["items"]) {
            oss << idx++ << ". " << item.value("name", "") << "\n";
            oss << "   Type: " << item.value("type", "") << "\n";
            oss << "   Trash path: " << item.value("path", "") << "\n";
            oss << "   Original path: " << item.value("origin_path", "—") << "\n";
            oss << "   Created: " << item.value("created", "") << "\n";
            oss << "   Deleted: " << item.value("deleted", "") << "\n";
            if (item.value("type", "") == "file" && item.contains("size")) {
                oss << "   Size: " << formatBytes(item["size"].get<std::uint64_t>()) << "\n";
            }
            oss << "\n";
        }
    } else {
        oss << "Trash is empty or could not retrieve contents.\n";
    }
    return oss.str();
}

std::string format_utils::formatResourceInfo(const nlohmann::json& info) {
    std::ostringstream oss;
    oss << "Name: " << info.value("name", "") << "\n";
    oss << "Path: " << info.value("path", "") << "\n";
    oss << "Type: " << info.value("type", "") << "\n";
    oss << "Size: ";
    if (info.contains("size")) {
        oss << formatBytes(info["size"].get<std::uint64_t>());
    } else {
        oss << "—";
    }
    oss << "\n";
    oss << "Created: " << info.value("created", "") << "\n";
    oss << "Modified: " << info.value("modified", "") << "\n";
    oss << "Public URL: " << (info.contains("public_url") &&
    !info["public_url"].is_null() ? info["public_url"].get<std::string>() : "—") << "\n";
    oss << "MD5: " << info.value("md5", "—") << "\n";
    return oss.str();
}