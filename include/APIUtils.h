#ifndef YANDEX_DISK_CPP_CLIENT_APIUTILS_H
#define YANDEX_DISK_CPP_CLIENT_APIUTILS_H
#pragma once

#include <map>
#include <string>

class HttpClient;

namespace api_utils {
    [[nodiscard]] std::string buildUrl(const std::string& endpoint,
                         const std::map<std::string, std::string>& params,
                         const HttpClient& client);

    [[nodiscard]] std::string buildUrl(const std::string& endpoint,
                         const std::string& path,
                         const std::string& extra_params,
                         const HttpClient& client);

    void checkApiError(const std::string& response);

    [[nodiscard]] std::string extractLinkByKey(const std::string& response,
                                 const std::string& key,
                                 const std::string& error_message);
}
#endif //YANDEX_DISK_CPP_CLIENT_APIUTILS_H
