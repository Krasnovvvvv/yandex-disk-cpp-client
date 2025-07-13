#include "YandexDiskClient.h"
#include <curl/curl.h>
#include <stdexcept>
#include <filesystem>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}


YandexDiskClient::YandexDiskClient(const std::string& oauth_token)
        : token(oauth_token) {}

std::string YandexDiskClient::buildUrl(
        const std::string& endpoint,
        const std::string& path,
        const std::string& extraParams
) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("curl_easy_init() failed");

    char* escaped = curl_easy_escape(curl, path.c_str(), 0);
    if (!escaped) {
        curl_easy_cleanup(curl);
        throw std::runtime_error("curl_easy_escape() failed");
    }

    std::string url = endpoint + escaped + extraParams;
    curl_free(escaped);
    curl_easy_cleanup(curl);

    return url;
}

std::string YandexDiskClient::getLinkByKey(
        const std::string& path,
        const std::string& endpoint,
        const std::string& key,
        const std::string& extraParams,
        const std::string& errorMsg
) {
    std::string url = buildUrl(endpoint, path, extraParams);
    std::string resp = performRequest(url);
    auto json = nlohmann::json::parse(resp);

    if (json.contains(key) && !json[key].is_null())
        return json[key].get<std::string>();
    else if (json.contains("error"))
        throw std::runtime_error("Yandex.Disk API error: " + json["error"].get<std::string>());
    else
        throw std::runtime_error(errorMsg);
}

std::string YandexDiskClient::performRequest(
        const std::string& url,
        const std::string& method,
        long* http_code /* = nullptr */)
{
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
    if (method == "DELETE") curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

    CURLcode res = curl_easy_perform(curl);

    long code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    if (http_code) *http_code = code;

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) throw std::runtime_error(curl_easy_strerror(res));
    return response;
}

nlohmann::json YandexDiskClient::getResourceList(const std::string& path) {
    std::string url = buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources?path=",
            path,
            ""
    );
    std::string resp = performRequest(url);
    return nlohmann::json::parse(resp);
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

bool YandexDiskClient::publish(const std::string& path) {
    std::string url = buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources/publish?path=",
            path,
            ""
    );
    std::string resp = performRequest(url, "PUT");
    auto json = nlohmann::json::parse(resp);

    if (json.contains("error")) {
        std::string msg = "Yandex.Disk API error";
        if (json.contains("message") && json["message"].is_string())
            msg += ": " + json["message"].get<std::string>();
        else if (json["error"].is_string())
            msg += ": " + json["error"].get<std::string>();
        throw std::runtime_error(msg);
    }
    return true;
}

std::string YandexDiskClient::getPublicDownloadLink(const std::string& path) {
    return getLinkByKey(
            path,
            "https://cloud-api.yandex.net/v1/disk/resources?path=",
            "public_url",
            "",
            "The file or directory has not been published! "
            "Use publish() method to publish it."
    );
}

std::string YandexDiskClient::getUploadUrl(const std::string& upload_disk_path) {
    return getLinkByKey(
            upload_disk_path,
            "https://cloud-api.yandex.net/v1/disk/resources/upload?path=",
            "href",
            "&overwrite=true",
            "Upload URL not found in API response."
    );
}

std::string YandexDiskClient::getDownloadUrl(const std::string& download_disk_path) {
    return getLinkByKey(
            download_disk_path,
            "https://cloud-api.yandex.net/v1/disk/resources/download?path=",
            "href",
            "",
            "Download URL not found in API response."
    );
}

std::string YandexDiskClient::makeUploadDiskPath(
        const std::string& upload_disk_path,
        const std::string& local_path) {

    std::filesystem::path diskPath(upload_disk_path);
    if (diskPath.has_filename() && diskPath.extension() != "") {
#if defined(_WIN32)
        return diskPath.u8string();
#else
        return diskPath.string();
#endif
    }

    std::filesystem::path localFile(local_path);

#if defined(_WIN32)
    return (diskPath / localFile.filename()).u8string();
#else
    return (diskPath / localFile.filename()).string();
#endif
}

std::string YandexDiskClient::makeLocalDownloadPath(
        const std::string& download_disk_path,
        const std::string& local_path) {
    std::filesystem::path localDir(local_path);
    std::filesystem::path diskFile(download_disk_path);

    if (localDir.has_filename() && localDir.extension() != "") {
#if defined(_WIN32)
        return localDir.u8string();
#else
        return localDir.string();
#endif
    }

#if defined(_WIN32)
    return (localDir / diskFile.filename()).u8string();
#else
    return (localDir / diskFile.filename()).string();
#endif
}

bool YandexDiskClient::uploadFile(
        const std::string& disk_dir,
        const std::string& local_path) {

    std::string upload_disk_path = makeUploadDiskPath(disk_dir, local_path);

    std::string url = getUploadUrl(upload_disk_path);

#if defined(_WIN32)
    FILE* file = _wfopen(std::filesystem::path(local_path).wstring().c_str(), L"rb");
#else
    FILE* file = fopen(local_path.c_str(), "rb");
#endif

    if (!file) {
        throw std::runtime_error("Couldn't open the file: " + local_path);
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        fclose(file);
        throw std::runtime_error("curl_easy_init() failed");
    }

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_PUT, 1L);
    curl_easy_setopt(curl, CURLOPT_READDATA, file);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)filesize);

    CURLcode res = curl_easy_perform(curl);

    fclose(file);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        throw std::runtime_error("File upload error: " +
                                 std::string(curl_easy_strerror(res)));
    }

    return true;
}

bool YandexDiskClient::downloadFile(
        const std::string& download_disk_path,
        const std::string& local_dir) {

    std::string local_path = makeLocalDownloadPath(
            download_disk_path,
            local_dir);

    std::string url = getDownloadUrl(download_disk_path);

#if defined(_WIN32)
    FILE* file = _wfopen(std::filesystem::path(local_path).wstring().c_str(), L"wb");
#else
    FILE* file = fopen(local_path.c_str(), "wb");
#endif
    if (!file) {
        throw std::runtime_error("Failed to create a file: " + local_path);
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        fclose(file);
        throw std::runtime_error("curl_easy_init() failed");
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, nullptr);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);


    CURLcode res = curl_easy_perform(curl);

    fclose(file);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        throw std::runtime_error("File download error: " +
        std::string(curl_easy_strerror(res)));
    }

    return true;
}

bool YandexDiskClient::deleteFile(const std::string& disk_path) {

    std::filesystem::path p(disk_path);
#if defined(_WIN32)
    std::string url = buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources?path=",
            p.u8string(),
            ""
            );
#else
    std::string url = buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources?path=",
            p.string(),
            ""
            );
#endif

    std::string resp = performRequest(url, "DELETE");

    auto json = nlohmann::json::parse(resp, nullptr, false);
    if(json.is_object() && json.contains("error")) {
        std::string msg = "Yandex.Disk API error";
        if(json.contains("message") && json["message"].is_string())
            msg += ": " + json["message"].get<std::string>();
        else if (json["error"].is_string())
            msg += ": " + json["error"].get<std::string>();
        throw std::runtime_error(msg);
    }

    return true;
}






