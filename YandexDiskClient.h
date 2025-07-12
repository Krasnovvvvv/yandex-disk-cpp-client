#ifndef YANDEX_DISK_CPP_CLIENT_YANDEXDISKCLIENT_H
#define YANDEX_DISK_CPP_CLIENT_YANDEXDISKCLIENT_H


#pragma once
#include <string>
#include <nlohmann/json.hpp>

class YandexDiskClient {
public:
    explicit YandexDiskClient(const std::string& );

    nlohmann::json getResourceList(const std::string& path ="/");
    std::string formatResourceList(const nlohmann::json& );
    bool publish(const std::string& );
    std::string getPublicDownloadLink(const std::string& );
    bool uploadFile(const std::string& , const std::string& );
    bool downloadFile(const std::string& download_disk_path, const std::string& local_path);


private:
    std::string token;
    std::string performRequest(const std::string& , const std::string& method = "GET");
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


};


#endif //YANDEX_DISK_CPP_CLIENT_YANDEXDISKCLIENT_H
