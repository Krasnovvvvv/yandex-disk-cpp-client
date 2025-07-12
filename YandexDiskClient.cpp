#include "YandexDiskClient.h"
#include <curl/curl.h>
#include <stdexcept>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

YandexDiskClient::YandexDiskClient(const std::string& oauth_token)
        : token(oauth_token) {}

std::string YandexDiskClient::performRequest(const std::string& url, const std::string& method) {
    CURL* curl = curl_easy_init();
    std::string response;
    if (!curl) throw std::runtime_error("curl_easy_init() failed");

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: OAuth " + token).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    if (method == "PUT") curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) throw std::runtime_error(curl_easy_strerror(res));
    return response;
}

nlohmann::json YandexDiskClient::getResourceList(const std::string& path) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("curl_easy_init() failed");

    char* escaped = curl_easy_escape(curl, path.c_str(), 0);
    if (!escaped) {
        curl_easy_cleanup(curl);
        throw std::runtime_error("curl_easy_escape() failed");
    }

    std::string url = "https://cloud-api.yandex.net/v1/disk/resources?path=";
    url += escaped;

    curl_free(escaped);
    curl_easy_cleanup(curl);

    std::string resp = performRequest(url);
    return nlohmann::json::parse(resp);
}

bool YandexDiskClient::publish(const std::string& path) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("curl_easy_init() failed");

    char* escaped = curl_easy_escape(curl, path.c_str(), 0);
    if (!escaped) {
        curl_easy_cleanup(curl);
        throw std::runtime_error("curl_easy_escape() failed");
    }

    std::string url = "https://cloud-api.yandex.net/v1/disk/resources/publish?path=";
    url += escaped;

    curl_free(escaped);
    curl_easy_cleanup(curl);

    performRequest(url, "PUT");
    return true;
}

std::string YandexDiskClient::getPublicDownloadLink(const std::string& path) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("curl_easy_init() failed");

    char* escaped = curl_easy_escape(curl, path.c_str(), 0);
    if (!escaped) {
        curl_easy_cleanup(curl);
        throw std::runtime_error("curl_easy_escape() failed");
    }

    std::string url = "https://cloud-api.yandex.net/v1/disk/resources?path=";
    url += escaped;

    curl_free(escaped);
    curl_easy_cleanup(curl);

    std::string resp = performRequest(url);
    auto json = nlohmann::json::parse(resp);
    if (json.contains("public_url") && !json["public_url"].is_null())
        return json["public_url"].get<std::string>();
    else
        throw std::runtime_error("The file or directory has not been published! "
                                 "Use publish() method to publish it.");
}



std::string YandexDiskClient::formatResourceList(const nlohmann::json& json) {
    std::ostringstream oss;
    int idx = 1;
    for (const auto& item : json["_embedded"]["items"]) {
        oss << idx++ << ". " << item["name"].get<std::string>() << "\n";
        oss << "   Type: " << item["type"].get<std::string>() << "\n";
        oss << "   Path: " << item["path"].get<std::string>() << "\n";
        if (item.contains("public_url"))
            oss << "   Public URL: " << item["public_url"].get<std::string>() << "\n";
        else
            oss << "   Public URL: is missing\n";
        oss << "\n";
    }
    return oss.str();
}

std::string YandexDiskClient::getUploadUrl(const std::string& upload_disk_path) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("curl_easy_init() failed");
    char* escaped = curl_easy_escape(curl, upload_disk_path.c_str(), 0);
    if (!escaped) {
        curl_easy_cleanup(curl);
        throw std::runtime_error("curl_easy_escape() failed");
    }

    std::string url = "https://cloud-api.yandex.net/v1/disk/resources/upload?path=";
    url += escaped;
    url += "&overwrite=true";

    curl_free(escaped);
    curl_easy_cleanup(curl);

    std::string resp = performRequest(url);
    auto json = nlohmann::json::parse(resp);
    if (json.contains("href") && !json["href"].is_null())
        return json["href"].get<std::string>();
    else if (json.contains("error"))
        throw std::runtime_error("Yandex.Disk API error: " + json["error"].get<std::string>());
    else
        throw std::runtime_error("Upload URL not found in API response");
}






