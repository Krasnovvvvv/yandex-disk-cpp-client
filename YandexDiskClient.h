#ifndef YANDEX_DISK_CPP_CLIENT_YANDEXDISKCLIENT_H
#define YANDEX_DISK_CPP_CLIENT_YANDEXDISKCLIENT_H


#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <filesystem>

class YandexDiskClient {
public:
    explicit YandexDiskClient(const std::string& );

    nlohmann::json getResourceList(const std::string& path ="/");
    std::string formatResourceList(const nlohmann::json& );
    bool publish(const std::string& );
    std::string getPublicDownloadLink(const std::string& );
    bool uploadFile(const std::string& , const std::string& );
    bool downloadFile(const std::string& , const std::string& );
    bool deleteFile(const std::string& );


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

};


#endif //YANDEX_DISK_CPP_CLIENT_YANDEXDISKCLIENT_H
