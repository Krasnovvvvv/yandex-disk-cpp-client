#include "SearchUtils.h"

std::vector<std::string> search_utils::findPathsByName(
    const std::string &name,
    const std::string &start_path,
    std::function<nlohmann::json(const std::string &)> list_func,
    bool recursive) {
    std::vector<std::string> results;

    nlohmann::json resList = list_func(start_path);
    if (resList.contains("_embedded") && resList["_embedded"].contains("items")) {
        for (const auto& item : resList["_embedded"]["items"]) {
            if (item.value("name", "") == name) {
                results.push_back(item.value("path", ""));
            }
            if (recursive && item.value("type", "") == "dir") {
                auto subResults = findPathsByName(
                        name,
                        item.value("path", ""),
                        list_func,
                        recursive);
                results.insert(
                        results.end(),
                        subResults.begin(),
                        subResults.end());
            }
        }
    }
    return results;
}
