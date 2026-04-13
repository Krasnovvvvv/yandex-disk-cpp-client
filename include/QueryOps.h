#ifndef YANDEX_DISK_CPP_CLIENT_QUERYOPS_H
#define YANDEX_DISK_CPP_CLIENT_QUERYOPS_H
#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "HttpClient.h"

namespace query_ops {

    nlohmann::json getQuotaInfo(HttpClient& client);

    nlohmann::json getResourceList(HttpClient& client,
                                   const std::string& disk_path);

    nlohmann::json getResourceInfo(HttpClient& client,
                                   const std::string& disk_path);

    nlohmann::json getTrashResourceList(HttpClient& client,
                                        const std::string& trash_path);

    std::string getPublicDownloadLink(HttpClient& client,
                                      const std::string& disk_path);

    std::string getUploadUrl(HttpClient& client,
                             const std::string& upload_disk_path);

    std::string getDownloadUrl(HttpClient& client,
                               const std::string& download_disk_path_utf8);

    bool exists(HttpClient& client,
                const std::string& disk_path);

}
#endif //YANDEX_DISK_CPP_CLIENT_QUERYOPS_H
