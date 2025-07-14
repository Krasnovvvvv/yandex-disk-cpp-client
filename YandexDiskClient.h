#ifndef YANDEX_DISK_CPP_CLIENT_YANDEXDISKCLIENT_H
#define YANDEX_DISK_CPP_CLIENT_YANDEXDISKCLIENT_H


#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <map>

class YandexDiskClient {
public:
    explicit YandexDiskClient(const std::string& );

    nlohmann::json getQuotaInfo();
    std::string formatQuotaInfo(const nlohmann::json& json);
    nlohmann::json getResourceList(const std::string& path ="/");
    std::string formatResourceList(const nlohmann::json& );
    std::string getResourceInfo(const std::string& disk_path);
    bool publish(const std::string& );
    bool unpublish(const std::string& disk_path);
    std::string getPublicDownloadLink(const std::string& );
    bool uploadFile(const std::string& , const std::string& );
    bool downloadFile(const std::string& , const std::string& );
    bool uploadDirectory(
            const std::string& disk_path,
            const std::string& local_path);
    bool downloadDirectory(
            const std::string& disk_path,
            const std::string& local_path);
    bool deleteFileOrDir(const std::string& );
    bool createDirectory(const std::string& disk_path);
    bool moveFileOrDir(
            const std::string& from_path,
            const std::string& to_path,
            bool overwrite = false
            );
    bool renameFileOrDir(
            const std::string& disk_path,
            const std::string& new_name,
            bool overwrite = false);

    bool exists(const std::string& disk_path);

private:
    std::string token;
    std::string performRequest(const std::string& ,
                               const std::string& method = "GET",
                               long* http_code  = nullptr);
    std::string getUploadUrl(const std::string& );
    std::string getDownloadUrl(const std::string& );

    std::string getLinkByKey(
            const std::string& ,
            const std::string& ,
            const std::string& ,
            const std::string& ,
            const std::string&
    );

    std::string buildUrl(
            const std::string& endpoint,
            const std::map<std::string, std::string>& params
    );

    std::string buildUrl(
            const std::string& ,
            const std::string& ,
            const std::string&
    );

    std::string makeUploadDiskPath(
            const std::string& ,
            const std::string& );

    std::string makeLocalDownloadPath(
            const std::string& ,
            const std::string& );

    std::string makeDiskPath(const std::string& disk_path);

    void checkApiError(const std::string& response);

};


#endif //YANDEX_DISK_CPP_CLIENT_YANDEXDISKCLIENT_H
