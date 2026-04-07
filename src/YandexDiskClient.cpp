#include "YandexDiskClient.h"
#include "HttpClient.h"
#include "PathUtils.h"
#include <stdexcept>
#include <filesystem>
#include <map>
#include <iomanip>
#include <sstream>

namespace {
    std::string formatBytes(std::uint64_t bytes) {
        std::ostringstream oss;
        auto value = static_cast<double>(bytes);
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int i = 0;
        while (value >= 1024 && i < 4) {
            value /= 1024;
            ++i;
        }
        oss << std::fixed << std::setprecision(2) << value << " " << units[i];
        return oss.str();
    }
}

YandexDiskClient::YandexDiskClient(const std::string& oauth_token)
        : http_(oauth_token) {}

std::string YandexDiskClient::buildUrl(
        const std::string& endpoint,
        const std::map<std::string, std::string>& params
) {
    std::string url = endpoint;
    bool first = true;

    for (const auto& [key, value] : params) {
        url += (first ? "?" : "&");
        url += key + "=" + http_.urlEncode(value);
        first = false;
    }

    return url;
}

std::string YandexDiskClient::buildUrl(
        const std::string& endpoint,
        const std::string& path,
        const std::string& extraParams
) {
    return endpoint + http_.urlEncode(path) + extraParams;
}

std::string YandexDiskClient::getLinkByKey(
        const std::string& path,
        const std::string& endpoint,
        const std::string& key,
        const std::string& extraParams,
        const std::string& errorMsg
) {
    std::string url = buildUrl(endpoint, path, extraParams);
    auto response = http_.request(url);
    std::string resp = response.body;
    checkApiError(resp);
    auto json = nlohmann::json::parse(resp);

    if (json.contains(key) && !json[key].is_null())
        return json[key].get<std::string>();
    else if (json.contains("error"))
        throw std::runtime_error("Yandex.Disk API error: " + json["error"].get<std::string>());
    else
        throw std::runtime_error(errorMsg);
}

nlohmann::json YandexDiskClient::getQuotaInfo() {
    std::string url = buildUrl("https://cloud-api.yandex.net/v1/disk", {});
    auto response = http_.request(url, "GET");
    std::string resp = response.body;
    checkApiError(resp);
    return nlohmann::json::parse(resp);
}

std::string YandexDiskClient::formatQuotaInfo(const nlohmann::json& quota) {
        std::ostringstream oss;
    oss << "Total space: " << formatBytes(quota["total_space"].get<uint64_t>()) << "\n";
    oss << "Used: " << formatBytes(quota["used_space"].get<uint64_t>()) << "\n";
    oss << "In trash: " << formatBytes(quota["trash_size"].get<uint64_t>()) << "\n";
    return oss.str();
}

nlohmann::json YandexDiskClient::getResourceList(const std::string& disk_path /* = "/" */) {
    const std::string path_utf8 = path_utils::makeDiskPath(disk_path);
    std::string url = buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources?path=",
            path_utf8,
            ""
    );
    auto response = http_.request(url);
    std::string resp = response.body;
    checkApiError(resp);
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
            {"path", path_utils::makeDiskPath(disk_path)}
    };
    std::string url = buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources",
            params);

    auto response = http_.request(url, "GET");
    std::string resp = response.body;
    checkApiError(resp);

    nlohmann::json info = nlohmann::json::parse(resp);

    std::ostringstream oss;
    oss << "Name: " << info.value("name", "") << "\n";
    oss << "Path: " << info.value("path", "") << "\n";
    oss << "Type: " << info.value("type", "") << "\n";
    oss << "Size: ";
    if (info.contains("size")) {
        oss << formatBytes(info["size"].get<std::uint64_t>());
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
    const std::string path_utf8 = path_utils::makeDiskPath(path);
    std::string url = buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources/publish?path=",
            path_utf8,
            ""
    );
    auto response = http_.request(url, "PUT");
    std::string resp = response.body;
    checkApiError(resp);

    return true;
}

bool YandexDiskClient::unpublish(const std::string& disk_path) {

    std::map<std::string, std::string> params = {
            {"path", path_utils::makeDiskPath(disk_path)}
    };

    std::string url = buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources/unpublish",
            params
            );

    auto response = http_.request(url, "PUT");
    std::string resp = response.body;
    checkApiError(resp);

    return true;
}

std::string YandexDiskClient::getPublicDownloadLink(const std::string& disk_path) {
    const std::string path_utf8 = path_utils::makeDiskPath(disk_path);
    return getLinkByKey(
            path_utf8,
            "https://cloud-api.yandex.net/v1/disk/resources?path=",
            "public_url",
            "",
            "The file or directory has not been published! "
            "Use publish() method to publish it."
    );
}

std::string YandexDiskClient::getUploadUrl(const std::string& upload_disk_path) {
    const std::string path_utf8 = path_utils::makeDiskPath(upload_disk_path);
    return getLinkByKey(
            path_utf8,
            "https://cloud-api.yandex.net/v1/disk/resources/upload?path=",
            "href",
            "&overwrite=true",
            "Upload URL not found in API response."
    );
}

std::string YandexDiskClient::getDownloadUrl(const std::string& download_disk_path) {
    const std::string path_utf8 = path_utils::makeDiskPath(download_disk_path);
    return getLinkByKey(
            path_utf8,
            "https://cloud-api.yandex.net/v1/disk/resources/download?path=",
            "href",
            "",
            "Download URL not found in API response."
    );
}

bool YandexDiskClient::uploadFile(
        const std::string& disk_dir,
        const std::string& local_path) {

    std::string upload_disk_path = path_utils::makeUploadDiskPath(disk_dir, local_path);

    std::string url = getUploadUrl(upload_disk_path);

    http_.uploadFileByUrl(url, local_path);

    return true;
}

bool YandexDiskClient::downloadFile(
        const std::string& download_disk_path,
        const std::string& local_dir)
{

    std::map<std::string, std::string> params = {
            {"path", path_utils::makeDiskPath(download_disk_path)}
    };
    std::string info_url = buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources",
            params);
    auto response = http_.request(info_url, "GET");
    std::string info_resp = response.body;
    checkApiError(info_resp);
    nlohmann::json meta = nlohmann::json::parse(info_resp);

    if (meta.value("type", "") == "dir") {
        throw std::runtime_error("Cannot download: '" +
        download_disk_path + "' is a directory, not a file.");
    }

    std::string local_path = path_utils::makeLocalDownloadPath(download_disk_path, local_dir);
    std::string url = getDownloadUrl(download_disk_path);

    http_.downloadToFile(url, local_path);

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

    std::string utf8_disk_path = path_utils::makeDiskPath(disk_path);

    std::string url = buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources?path=",
            utf8_disk_path,
            ""
            );

    auto response = http_.request(url, "DELETE");
    std::string resp = response.body;
    checkApiError(resp);

    return true;
}

bool YandexDiskClient::createDirectory(const std::string& disk_path) {

    std::string utf8_disk_path = path_utils::makeDiskPath(disk_path);

    std::string url = buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources?path=",
            utf8_disk_path,
            ""
    );

    auto response = http_.request(url, "PUT");
    std::string resp = response.body;
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

    std::string from_utf8 = path_utils::makeDiskPath(from_fs.string());
    std::string to_utf8 = path_utils::makeDiskPath(to_fs.string());

    std::map<std::string, std::string> params = {
            {"from", from_utf8},
            {"path", to_utf8}
    };
    if (overwrite) {
        params["overwrite"] = "true";
    }

    std::string url = buildUrl("https://cloud-api.yandex.net/v1/disk/resources/move", params);

    auto response = http_.request(url, "POST");
    std::string resp = response.body;
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

bool YandexDiskClient::exists(const std::string& disk_path) {
    try {
        std::map<std::string, std::string> params = {
                {"path", path_utils::makeDiskPath(disk_path)}
        };
        std::string url = buildUrl(
                "https://cloud-api.yandex.net/v1/disk/resources",
                params
        );

        auto response = http_.request(url, "GET");

        return response.status_code == 200;
    } catch (const std::exception& ex) {
        return false;
    }
}

nlohmann::json YandexDiskClient::getTrashResourceList(const std::string& trash_path /* = "trash:/" */) {
    std::map<std::string, std::string> params = {
            {"path", path_utils::makeDiskPath(trash_path)}
    };
    std::string url = buildUrl(
            "https://cloud-api.yandex.net/v1/disk/trash/resources",
            params
    );
    auto response = http_.request(url, "GET");
    std::string resp = response.body;
    checkApiError(resp);
    return nlohmann::json::parse(resp);
}

std::string YandexDiskClient::formatTrashResourceList(const nlohmann::json& json) {
    std::ostringstream oss;
    int idx = 1;
    if (json.contains("_embedded") &&
    json["_embedded"].contains("items") &&
    !json["_embedded"]["items"].empty()) {
        for (const auto& item : json["_embedded"]["items"]) {
            oss << idx++ << ". " << item.value("name", "") << "\n";
            oss << "   Type: " << item.value("type", "") << "\n";
            oss << "   Trash path: " << item.value("path", "") << "\n";
            oss << "   Original path: " << item.value("origin_path", "—") << "\n";
            oss << "   Created: " << item.value("created", "") << "\n";
            oss << "   Deleted: " << item.value("deleted", "") << "\n";
            if (item.value("type", "") == "file" && item.contains("size")) {
                oss << "   Size: " << formatBytes(item["size"].get<std::uint64_t>()) << "\n";
            }
            oss << "\n";
        }
    } else {
        oss << "Trash is empty or could not retrieve contents.\n";
    }
    return oss.str();
}

bool YandexDiskClient::restoreFromTrash(const std::string& trash_path) {
    std::map<std::string, std::string> params = {
            {"path", path_utils::makeDiskPath(trash_path)}
    };
    std::string url = buildUrl(
            "https://cloud-api.yandex.net/v1/disk/trash/resources/restore",
            params
    );
    auto response = http_.request(url, "PUT");
    std::string resp = response.body;
    checkApiError(resp);
    return true;
}

bool YandexDiskClient::deleteFromTrash(const std::string& trash_path) {
    std::map<std::string, std::string> params = {
            {"path", path_utils::makeDiskPath(trash_path)}
    };
    std::string url = buildUrl(
            "https://cloud-api.yandex.net/v1/disk/trash/resources",
            params
    );
    auto response = http_.request(url, "DELETE");
    std::string resp = response.body;
    checkApiError(resp);
    return true;
}

bool YandexDiskClient::emptyTrash() {
    std::string url = "https://cloud-api.yandex.net/v1/disk/trash/resources?path=";
    auto response = http_.request(url, "DELETE");
    std::string resp = response.body;
    checkApiError(resp);
    return true;
}

std::vector<std::string> YandexDiskClient::findPathsByName(
        const std::string& name,
        const std::string& start_path,
        std::function<nlohmann::json(const std::string&)> listFunc,
        bool recursive /* = true */)
{
    std::vector<std::string> results;
    nlohmann::json resList = listFunc(start_path);
    if (resList.contains("_embedded") && resList["_embedded"].contains("items")) {
        for (const auto& item : resList["_embedded"]["items"]) {
            if (item.value("name", "") == name) {
                results.push_back(item.value("path", ""));
            }
            if (recursive && item.value("type", "") == "dir") {
                auto subResults = findPathsByName(
                        name,
                        item.value("path", ""),
                        listFunc,
                        recursive);
                results.insert(
                        results.end(),
                        subResults.begin(),
                        subResults.end());
            }
        }
    }
    return results;
}

std::vector<std::string> YandexDiskClient::findTrashPathByName(const std::string& name) {
    auto listTrash =
            [this](const std::string& path) -> nlohmann::json {
        return getTrashResourceList(path);
    };

    return findPathsByName(name, "/", listTrash, false);
}

std::vector<std::string> YandexDiskClient::findResourcePathByName(
        const std::string& name,
        const std::string& start_path /* = "/" */) {
    auto listDisk =
            [this](const std::string& path) -> nlohmann::json {
        return getResourceList(path);
    };

    return findPathsByName(
            name,
            start_path.empty() ? "/" : start_path,
            listDisk,
            true);
}