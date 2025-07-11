#ifndef YANDEX_DISK_CPP_CLIENT_YANDEXDISKCLIENT_H
#define YANDEX_DISK_CPP_CLIENT_YANDEXDISKCLIENT_H


#pragma once
#include <string>
#include <nlohmann/json.hpp>

class YandexDiskClient {
public:
    explicit YandexDiskClient(const std::string& oauth_token);

    nlohmann::json getResourceList(const std::string& path ="/");
    bool publish(const std::string& path);
    std::string getPublicDownloadLink(const std::string& path);
    std::string formatResourceList(const nlohmann::json& json);

private:
    std::string token;
    std::string performRequest(const std::string& url, const std::string& method = "GET");

};


#endif //YANDEX_DISK_CPP_CLIENT_YANDEXDISKCLIENT_H
