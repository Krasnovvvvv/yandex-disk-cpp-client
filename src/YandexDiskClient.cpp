#include "YandexDiskClient.h"
#include "HttpClient.h"
#include "PathUtils.h"
#include "APIUtils.h"
#include "FormatUtils.h"
#include "SearchUtils.h"
#include "DirectoryOps.h"
#include "ResourceOps.h"
#include "QueryOps.h"
#include <stdexcept>
#include <filesystem>
#include <map>
#include <iomanip>
#include <sstream>

YandexDiskClient::YandexDiskClient(const std::string& oauth_token)
        : http_(oauth_token) {}

nlohmann::json YandexDiskClient::getQuotaInfo() {
    return query_ops::getQuotaInfo(http_);
}

nlohmann::json YandexDiskClient::getResourceList(const std::string& disk_path /* = "/" */) {
    return query_ops::getResourceList(http_, path_utils::makeDiskPath(disk_path));
}

std::string YandexDiskClient::getResourceInfo(const std::string& disk_path) {
    auto info = query_ops::getResourceInfo(http_, path_utils::makeDiskPath(disk_path));
    return format_utils::formatResourceInfo(info);
}

bool YandexDiskClient::publish(const std::string& path) {
    const std::string path_utf8 = path_utils::makeDiskPath(path);
    return resource_ops::publish(http_, path_utf8);
}

bool YandexDiskClient::unpublish(const std::string& disk_path) {
    const std::string utf8_disk_path = path_utils::makeDiskPath(disk_path);
    return resource_ops::unpublish(http_, utf8_disk_path);
}

std::string YandexDiskClient::getPublicDownloadLink(const std::string& disk_path) {
    return query_ops::getPublicDownloadLink(http_, path_utils::makeDiskPath(disk_path));
}

std::string YandexDiskClient::getUploadUrl(const std::string& upload_disk_path) {
    return query_ops::getUploadUrl(http_, path_utils::makeDiskPath(upload_disk_path));
}

std::string YandexDiskClient::getDownloadUrl(const std::string& download_disk_path) {
    return query_ops::getDownloadUrl(http_, path_utils::makeDiskPath(download_disk_path));
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
    const std::string path_utf8 = path_utils::makeDiskPath(download_disk_path);
    const nlohmann::json meta = query_ops::getResourceInfo(http_, path_utf8);

    if (meta.value("type", "") == "dir") {
        throw std::runtime_error(
            "Cannot download: '" + download_disk_path + "' is a directory, not a file.");
    }

    const std::string local_path =
        path_utils::makeLocalDownloadPath(download_disk_path, local_dir);

    const std::string url = query_ops::getDownloadUrl(http_, path_utf8);
    http_.downloadToFile(url, local_path);

    return true;
}

bool YandexDiskClient::uploadDirectory(
        const std::string& disk_path,
        const std::string& local_path)
{
    directory_ops::uploadDirectory(
        disk_path,
        local_path,
        [this](const std::string& remote_dir) {
            this->createDirectory(remote_dir);
        },
        [this](const std::string& remote_path, const std::string& local_file) {
            this->uploadFile(remote_path, local_file);
        });

    return true;
}

bool YandexDiskClient::downloadDirectory(
        const std::string& disk_path,
        const std::string& local_path)
{
    directory_ops::downloadDirectory(
        disk_path,
        local_path,
        [this](const std::string& remote_dir) -> nlohmann::json {
            return this->getResourceList(remote_dir);
        },
        [this](const std::string& remote_file, const std::string& local_dir) {
            this->downloadFile(remote_file, local_dir);
        });

    return true;
}

bool YandexDiskClient::deleteFileOrDir(const std::string& disk_path) {
    const std::string utf8_disk_path = path_utils::makeDiskPath(disk_path);
    return resource_ops::deleteFileOrDir(http_, utf8_disk_path);
}

bool YandexDiskClient::createDirectory(const std::string& disk_path) {
    const std::string utf8_disk_path = path_utils::makeDiskPath(disk_path);
    return resource_ops::createDirectory(http_, utf8_disk_path);
}

bool YandexDiskClient::moveFileOrDir(
        const std::string& from_path,
        const std::string& to_path,
        bool overwrite) {
    std::filesystem::path from_fs(from_path);
    std::filesystem::path to_fs =
        path_utils::resolveMoveDestination(
            from_fs,
            std::filesystem::path(to_path),
            to_path
        );

#if defined(_WIN32)
    const std::string from_normalized = from_fs.u8string();
    const std::string to_normalized   = to_fs.u8string();
#else
    const std::string from_normalized = from_fs.string();
    const std::string to_normalized   = to_fs.string();
#endif

    return resource_ops::moveFileOrDir(
        http_,
        path_utils::makeDiskPath(from_normalized),
        path_utils::makeDiskPath(to_normalized),
        overwrite
    );
}

bool YandexDiskClient::renameFileOrDir(
        const std::string& disk_path,
        const std::string& new_name,
        bool overwrite /* = false */) {

    std::filesystem::path from_fs(disk_path);
    std::filesystem::path dst_fs = from_fs.parent_path() / new_name;

#if defined(_WIN32)
    const std::string dst_normalized = dst_fs.u8string();
#else
    const std::string dst_normalized = dst_fs.string();
#endif

    return moveFileOrDir(disk_path, dst_normalized, overwrite);
}

bool YandexDiskClient::exists(const std::string& disk_path) {
    return query_ops::exists(http_, path_utils::makeDiskPath(disk_path));
}

nlohmann::json YandexDiskClient::getTrashResourceList(const std::string& trash_path /* = "trash:/" */) {
    return query_ops::getTrashResourceList(http_, path_utils::makeDiskPath(trash_path));
}

bool YandexDiskClient::restoreFromTrash(const std::string& trash_path) {
    const std::string path_utf8 = path_utils::makeDiskPath(trash_path);
    return resource_ops::restoreFromTrash(http_, path_utf8);
}

bool YandexDiskClient::deleteFromTrash(const std::string& trash_path) {
    const std::string path_utf8 = path_utils::makeDiskPath(trash_path);
    return resource_ops::deleteFromTrash(http_, path_utf8);
}

bool YandexDiskClient::emptyTrash() {
    return resource_ops::emptyTrash(http_);
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