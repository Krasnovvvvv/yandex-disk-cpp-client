#include "APIUtils.h"

#include <stdexcept>
#include <nlohmann/json.hpp>

#include "HttpClient.h"

std::string api_utils::buildUrl(const std::string &endpoint,
                                const std::map<std::string, std::string> &params,
                                const HttpClient &client) {
    std::string url = endpoint;
    bool first = true;

    for (const auto& [key, value] : params) {
        url += (first ? "?" : "&");
        url += key + "=" + client.urlEncode(value);
        first = false;
    }

    return url;
}

std::string api_utils::buildUrl(const std::string &endpoint,
                                const std::string &path,
                                const std::string &extra_params,
                                const HttpClient &client) {
    return endpoint + client.urlEncode(path) + extra_params;
}

void api_utils::checkApiError(const std::string &response) {
    auto json = nlohmann::json::parse(response, nullptr, false);

    if(json.is_object() && json.contains("error")) {
        std::string msg = "Yandex.Disk API error";

        if(json.contains("message") && json["message"].is_string())
            msg += ": " + json["message"].get<std::string>();
        else if (json["error"].is_string())
            msg += ": " + json["error"].get<std::string>();
        throw std::runtime_error(msg);
    }
}

std::string api_utils::extractLinkByKey(const std::string &response,
                                        const std::string &key,
                                        const std::string &error_message) {
    auto json = nlohmann::json::parse(response);

    if (json.contains(key) && !json[key].is_null())
        return json[key].get<std::string>();
    if (json.contains("error"))
        throw std::runtime_error("Yandex.Disk API error: " + json["error"].get<std::string>());
    throw std::runtime_error(error_message);
}
