#ifndef YANDEX_DISK_CPP_CLIENT_RESOURCEOPS_H
#define YANDEX_DISK_CPP_CLIENT_RESOURCEOPS_H
#pragma once
#include <string>

class HttpClient;

namespace resource_ops {

    bool createDirectory(HttpClient& client, const std::string& disk_path);
    bool deleteFileOrDir(HttpClient& client, const std::string& disk_path);

    bool moveFileOrDir(HttpClient& client,
                       const std::string& from_path,
                       const std::string& to_path,
                       bool overwrite = false);

    bool publish(HttpClient& client, const std::string& disk_path);
    bool unpublish(HttpClient& client, const std::string& disk_path);

    bool restoreFromTrash(HttpClient& client, const std::string& trash_path);
    bool deleteFromTrash(HttpClient& client, const std::string& trash_path);
    bool emptyTrash(HttpClient& client);

}
#endif //YANDEX_DISK_CPP_CLIENT_RESOURCEOPS_H
