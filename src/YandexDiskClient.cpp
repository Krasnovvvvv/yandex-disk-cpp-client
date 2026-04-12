#include "YandexDiskClient.h"
#include "HttpClient.h"
#include "PathUtils.h"
#include "APIUtils.h"
#include "FormatUtils.h"
#include "SearchUtils.h"
#include <stdexcept>
#include <filesystem>
#include <map>
#include <iomanip>
#include <sstream>

YandexDiskClient::YandexDiskClient(const std::string& oauth_token)
        : http_(oauth_token) {}

nlohmann::json YandexDiskClient::getQuotaInfo() {
    std::string url = api_utils::buildUrl("https://cloud-api.yandex.net/v1/disk", {}, http_);
    auto response = http_.request(url, "GET");
    std::string resp = response.body;
    api_utils::checkApiError(resp);
    return nlohmann::json::parse(resp);
}

nlohmann::json YandexDiskClient::getResourceList(const std::string& disk_path /* = "/" */) {
    const std::string path_utf8 = path_utils::makeDiskPath(disk_path);
    std::string url = api_utils::buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources?path=",
            path_utf8,
            "",
            http_
    );
    auto response = http_.request(url);
    std::string resp = response.body;
    api_utils::checkApiError(resp);
    return nlohmann::json::parse(resp);
}

std::string YandexDiskClient::getResourceInfo(const std::string& disk_path) {

    std::map<std::string, std::string> params = {
            {"path", path_utils::makeDiskPath(disk_path)}
    };
    std::string url = api_utils::buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources",
            params,
            http_);

    auto response = http_.request(url, "GET");
    api_utils::checkApiError(response.body);

    return format_utils::formatResourceInfo(
        nlohmann::json::parse(response.body));
}

bool YandexDiskClient::publish(const std::string& path) {
    const std::string path_utf8 = path_utils::makeDiskPath(path);
    std::string url = api_utils::buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources/publish?path=",
            path_utf8,
            "",
            http_
    );
    auto response = http_.request(url, "PUT");
    std::string resp = response.body;
    api_utils::checkApiError(resp);

    return true;
}

bool YandexDiskClient::unpublish(const std::string& disk_path) {

    std::map<std::string, std::string> params = {
            {"path", path_utils::makeDiskPath(disk_path)}
    };

    std::string url = api_utils::buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources/unpublish",
            params,
            http_
            );

    auto response = http_.request(url, "PUT");
    std::string resp = response.body;
    api_utils::checkApiError(resp);

    return true;
}

std::string YandexDiskClient::getPublicDownloadLink(const std::string& disk_path) {
    const std::string path_utf8 = path_utils::makeDiskPath(disk_path);
    const std::string url = api_utils::buildUrl(
        "https://cloud-api.yandex.net/v1/disk/resources?path=",
        path_utf8,
        "",
        http_
    );

    auto response = http_.request(url);
    api_utils::checkApiError(response.body);

    return api_utils::extractLinkByKey(
        response.body,
        "public_url",
        "The file or directory has not been published! "
            "Use publish() method to publish it."
   );
}

std::string YandexDiskClient::getUploadUrl(const std::string& upload_disk_path) {
    const std::string path_utf8 = path_utils::makeDiskPath(upload_disk_path);
    const std::string url = api_utils::buildUrl(
        "https://cloud-api.yandex.net/v1/disk/resources/upload?path=",
        path_utf8,
        "&overwrite=true",
        http_
    );

    auto response = http_.request(url);
    api_utils::checkApiError(response.body);

    return api_utils::extractLinkByKey(
        response.body,
        "href",
        "Upload URL not found in API response."
    );
}

std::string YandexDiskClient::getDownloadUrl(const std::string& download_disk_path) {
    const std::string path_utf8 = path_utils::makeDiskPath(download_disk_path);
    const std::string url = api_utils::buildUrl(
        "https://cloud-api.yandex.net/v1/disk/resources/download?path=",
        path_utf8,
        "",
        http_
    );

    auto response = http_.request(url);
    api_utils::checkApiError(response.body);

    return api_utils::extractLinkByKey(
            response.body,
            "href",
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
    std::string info_url = api_utils::buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources",
            params,
            http_);
    auto response = http_.request(info_url, "GET");
    std::string info_resp = response.body;
    api_utils::checkApiError(info_resp);
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

    std::string url = api_utils::buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources?path=",
            utf8_disk_path,
            "",
            http_);

    auto response = http_.request(url, "DELETE");
    std::string resp = response.body;
    api_utils::checkApiError(resp);

    return true;
}

bool YandexDiskClient::createDirectory(const std::string& disk_path) {

    std::string utf8_disk_path = path_utils::makeDiskPath(disk_path);

    std::string url = api_utils::buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources?path=",
            utf8_disk_path,
            "",
            http_
    );

    auto response = http_.request(url, "PUT");
    std::string resp = response.body;
    api_utils::checkApiError(resp);

    return true;
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

    std::string url = api_utils::buildUrl("https://cloud-api.yandex.net/v1/disk/resources/move",
                                          params, http_);

    auto response = http_.request(url, "POST");
    std::string resp = response.body;
    api_utils::checkApiError(resp);

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
        std::string url = api_utils::buildUrl(
                "https://cloud-api.yandex.net/v1/disk/resources",
                params,
                http_
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
    std::string url = api_utils::buildUrl(
            "https://cloud-api.yandex.net/v1/disk/trash/resources",
            params,
            http_
    );
    auto response = http_.request(url, "GET");
    std::string resp = response.body;
    api_utils::checkApiError(resp);
    return nlohmann::json::parse(resp);
}

bool YandexDiskClient::restoreFromTrash(const std::string& trash_path) {
    std::map<std::string, std::string> params = {
            {"path", path_utils::makeDiskPath(trash_path)}
    };
    std::string url = api_utils::buildUrl(
            "https://cloud-api.yandex.net/v1/disk/trash/resources/restore",
            params,
            http_
    );
    auto response = http_.request(url, "PUT");
    std::string resp = response.body;
    api_utils::checkApiError(resp);
    return true;
}

bool YandexDiskClient::deleteFromTrash(const std::string& trash_path) {
    std::map<std::string, std::string> params = {
            {"path", path_utils::makeDiskPath(trash_path)}
    };
    std::string url = api_utils::buildUrl(
            "https://cloud-api.yandex.net/v1/disk/trash/resources",
            params,
            http_
    );
    auto response = http_.request(url, "DELETE");
    std::string resp = response.body;
    api_utils::checkApiError(resp);
    return true;
}

bool YandexDiskClient::emptyTrash() {
    std::string url = "https://cloud-api.yandex.net/v1/disk/trash/resources?path=";
    auto response = http_.request(url, "DELETE");
    std::string resp = response.body;
    api_utils::checkApiError(resp);
    return true;
}

std::vector<std::string> YandexDiskClient::findTrashPathByName(const std::string& name) {
    auto listTrash =
            [this](const std::string& path) -> nlohmann::json {
        return getTrashResourceList(path);
    };

    return search_utils::findPathsByName(name, "/", listTrash, false);
}

std::vector<std::string> YandexDiskClient::findResourcePathByName(
        const std::string& name,
        const std::string& start_path /* = "/" */) {
    auto listDisk =
            [this](const std::string& path) -> nlohmann::json {
        return getResourceList(path);
    };

    return search_utils::findPathsByName(
            name,
            start_path.empty() ? "/" : start_path,
            listDisk,
            true);
}