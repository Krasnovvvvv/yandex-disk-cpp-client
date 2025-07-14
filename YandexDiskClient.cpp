#include "YandexDiskClient.h"
#include <curl/curl.h>
#include <stdexcept>
#include <filesystem>
#include <map>
#include <iomanip>
#include <sstream>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}


YandexDiskClient::YandexDiskClient(const std::string& oauth_token)
        : token(oauth_token) {}

std::string YandexDiskClient::buildUrl(
        const std::string& endpoint,
        const std::map<std::string, std::string>& params
) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("curl_easy_init() failed");

    std::string url = endpoint;
    bool first = true;
    for (const auto& [key, value] : params) {
        char* escaped = curl_easy_escape(curl, value.c_str(), 0);
        if (!escaped) {
            curl_easy_cleanup(curl);
            throw std::runtime_error("curl_easy_escape() failed");
        }
        url += (first ? "?" : "&");
        url += key + "=" + escaped;
        curl_free(escaped);
        first = false;
    }
    curl_easy_cleanup(curl);
    return url;
}

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

    if (method == "PUT") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    } else if (method == "DELETE") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
    } else if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
    }

    CURLcode res = curl_easy_perform(curl);

    long code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    if (http_code) *http_code = code;

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) throw std::runtime_error(curl_easy_strerror(res));
    return response;
}

nlohmann::json YandexDiskClient::getQuotaInfo() {
    std::string url = buildUrl("https://cloud-api.yandex.net/v1/disk", {});
    std::string resp = performRequest(url, "GET");
    checkApiError(resp);
    return nlohmann::json::parse(resp);
}

std::string YandexDiskClient::formatQuotaInfo(const nlohmann::json& quota) {
    auto formatSize = [](uint64_t bytes) -> std::string {
        std::ostringstream oss;
        double value = bytes;
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int i = 0;
        while (value >= 1024 && i < 4) {
            value /= 1024;
            ++i;
        }
        oss << std::fixed << std::setprecision(2) << value << " " << units[i];
        return oss.str();
    };

    std::ostringstream oss;
    oss << "Total space: " << formatSize(quota["total_space"].get<uint64_t>()) << "\n";
    oss << "Used: " << formatSize(quota["used_space"].get<uint64_t>()) << "\n";
    oss << "In trash: " << formatSize(quota["trash_size"].get<uint64_t>()) << "\n";
    return oss.str();
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

std::string YandexDiskClient::getResourceInfo(const std::string& disk_path) {

    std::map<std::string, std::string> params = {
            {"path", makeDiskPath(disk_path)}
    };
    std::string url = buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources",
            params);

    std::string resp = performRequest(url, "GET");
    checkApiError(resp);

    nlohmann::json info = nlohmann::json::parse(resp);

    std::ostringstream oss;
    oss << "Name: " << info.value("name", "") << "\n";
    oss << "Path: " << info.value("path", "") << "\n";
    oss << "Type: " << info.value("type", "") << "\n";
    oss << "Size: ";
    if (info.contains("size")) {
        double value = info["size"].get<uint64_t>();
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int i = 0;
        while (value >= 1024 && i < 4) {
            value /= 1024;
            ++i;
        }
        oss << std::fixed << std::setprecision(2) << value << " " << units[i];
    } else {
        oss << "—";
    }
    oss << "\n";
    oss << "Created: " << info.value("created", "") << "\n";
    oss << "Modified: " << info.value("modified", "") << "\n";
    oss << "Public URL: " << (info.contains("public_url") &&
    !info["public_url"].is_null() ? info["public_url"].get<std::string>() : "—") << "\n";
    oss << "MD5: " << info.value("md5", "—") << "\n";
    return oss.str();
}

bool YandexDiskClient::publish(const std::string& path) {
    std::string url = buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources/publish?path=",
            path,
            ""
    );
    std::string resp = performRequest(url, "PUT");
    checkApiError(resp);

    return true;
}

bool YandexDiskClient::unpublish(const std::string& disk_path) {

    std::map<std::string, std::string> params = {
            {"path", makeDiskPath(disk_path)}
    };

    std::string url = buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources/unpublish",
            params
            );

    std::string resp = performRequest(url, "PUT");
    checkApiError(resp);

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

std::string YandexDiskClient::makeDiskPath(const std::string& disk_path) {
    std::filesystem::path p(disk_path);
#if defined(_WIN32)
    return p.u8string();
#else
    return p.string();
#endif
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
        const std::string& local_dir)
{

    std::map<std::string, std::string> params = {
            {"path", makeDiskPath(download_disk_path)}
    };
    std::string info_url = buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources",
            params);
    std::string info_resp = performRequest(info_url, "GET");
    nlohmann::json meta = nlohmann::json::parse(info_resp);

    if (meta.value("type", "") == "dir") {
        throw std::runtime_error("Cannot download: '" +
        download_disk_path + "' is a directory, not a file.");
    }

    std::string local_path = makeLocalDownloadPath(download_disk_path, local_dir);
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

bool YandexDiskClient::uploadDirectory(
        const std::string& disk_path,
        const std::string& local_path)
{
    namespace fs = std::filesystem;

    if (!fs::exists(local_path) || !fs::is_directory(local_path)) {
        throw std::runtime_error("Local directory does not exist: " + local_path);
    }

    fs::path disk_fs(disk_path);
    fs::path local_fs(local_path);

    if (disk_fs.empty() || disk_fs == "/" ||
    !disk_fs.has_filename() ||
    disk_path.back() == '/' ||
    disk_path.back() == '\\') {
        disk_fs /= local_fs.filename();
    }

    createDirectory(disk_fs.generic_string());

    for (const auto& entry : fs::recursive_directory_iterator(local_fs)) {
        fs::path rel_path = fs::relative(entry.path(), local_fs);
        std::string disk_target = (disk_fs / rel_path).generic_string();

        if (entry.is_directory()) {
            createDirectory(disk_target);
        } else if (entry.is_regular_file()) {
            uploadFile(disk_target, entry.path().string());
        }
    }

    return true;
}

bool YandexDiskClient::downloadDirectory(
        const std::string& disk_path,
        const std::string& local_path)
{
    namespace fs = std::filesystem;

    nlohmann::json info = getResourceList(disk_path);
    if (!info.contains("_embedded") || !info["_embedded"].contains("items")) {
        throw std::runtime_error("Remote directory does not exist or is not a directory: " +
        disk_path);
    }

    fs::path local_fs(local_path);
    fs::path disk_fs(disk_path);

    if (!fs::exists(local_fs) || fs::is_directory(local_fs)) {
        fs::path folder_name = disk_fs.filename();
        if (folder_name.empty()) {
            folder_name = disk_fs.parent_path().filename();
        }
        local_fs /= folder_name;
    } else if (fs::exists(local_fs) && !fs::is_directory(local_fs)) {
        throw std::runtime_error("Local path exists and is not a directory: " +
        local_path);
    }

    fs::create_directories(local_fs);

    for (const auto& item : info["_embedded"]["items"]) {
        std::string name = item["name"].get<std::string>();
        std::string type = item["type"].get<std::string>();
        std::string remote_item_path = item["path"].get<std::string>();
        fs::path local_item_path = local_fs / name;

        if (type == "dir") {
            downloadDirectory(remote_item_path, local_item_path.string());
        } else if (type == "file") {
            downloadFile(remote_item_path, local_item_path.string());
        }
    }

    return true;
}

bool YandexDiskClient::deleteFileOrDir(const std::string& disk_path) {

    std::string utf8_disk_path = makeDiskPath(disk_path);

    std::string url = buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources?path=",
            utf8_disk_path,
            ""
            );

    std::string resp = performRequest(url, "DELETE");
    checkApiError(resp);

    return true;
}

bool YandexDiskClient::createDirectory(const std::string& disk_path) {

    std::string utf8_disk_path = makeDiskPath(disk_path);

    std::string url = buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources?path=",
            utf8_disk_path,
            ""
    );

    std::string resp = performRequest(url, "PUT");
    checkApiError(resp);

    return true;
}

void YandexDiskClient::checkApiError(const std::string& response) {
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

bool YandexDiskClient::moveFileOrDir(
        const std::string& from_path,
        const std::string& to_path,
        bool overwrite /* = false */
) {
    std::filesystem::path from_fs(from_path);
    std::filesystem::path to_fs(to_path);

    if (to_fs.parent_path().empty()) {
        to_fs = from_fs.parent_path() / to_fs;
    } else if (!to_fs.has_filename() || to_path.back() == '/' || to_path.back() == '\\') {
        to_fs /= from_fs.filename();
    }

    std::string from_utf8 = makeDiskPath(from_fs.string());
    std::string to_utf8 = makeDiskPath(to_fs.string());

    std::map<std::string, std::string> params = {
            {"from", from_utf8},
            {"path", to_utf8}
    };
    if (overwrite) {
        params["overwrite"] = "true";
    }

    std::string url = buildUrl("https://cloud-api.yandex.net/v1/disk/resources/move", params);

    std::string resp = performRequest(url, "POST");
    checkApiError(resp);

    return true;
}

bool YandexDiskClient::renameFileOrDir(
        const std::string& disk_path,
        const std::string& new_name,
        bool overwrite /* = false */) {

    std::filesystem::path disk(disk_path);
    std::filesystem::path dst = disk.parent_path() / new_name;
    return moveFileOrDir(disk_path, dst.generic_string(), overwrite);
}






