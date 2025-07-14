#ifndef YANDEX_DISK_CPP_CLIENT_YANDEXDISKCLIENT_H
#define YANDEX_DISK_CPP_CLIENT_YANDEXDISKCLIENT_H

#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <map>

/**
 * @brief C++ client for Yandex.Disk REST API.
 */
class YandexDiskClient {
public:
    /**
     * @brief Constructor. Initializes client with OAuth token.
     * @param oauth_token Yandex.Disk OAuth token.
     */
    explicit YandexDiskClient(const std::string& oauth_token);

    /**
     * @brief Get disk quota information (total, used, trash).
     * @return JSON object with quota info.
     * @throws std::runtime_error on API/network error.
     */
    nlohmann::json getQuotaInfo();

    /**
     * @brief Format quota information as human-readable string.
     * @param json JSON object from getQuotaInfo().
     * @return Formatted string.
     */
    std::string formatQuotaInfo(const nlohmann::json& json);

    /**
     * @brief Get list of files and folders at given path.
     * @param disk_path Path on Yandex.Disk (default: root "/").
     * @return JSON object with resource list.
     * @throws std::runtime_error on API/network error.
     */
    nlohmann::json getResourceList(const std::string& disk_path = "/");

    /**
     * @brief Format resource list as human-readable string.
     * @param json JSON object from getResourceList().
     * @return Formatted string.
     */
    std::string formatResourceList(const nlohmann::json& json);

    /**
     * @brief Get detailed information about a file or folder.
     * @param disk_path Path to file or folder on Yandex.Disk.
     * @return Formatted string with resource info.
     * @throws std::runtime_error on API/network error.
     */
    std::string getResourceInfo(const std::string& disk_path);

    /**
     * @brief Publish a file or folder (make it public).
     * @param path Path to file or folder on Yandex.Disk.
     * @return true on success.
     * @throws std::runtime_error on API/network error.
     */
    bool publish(const std::string& path);

    /**
     * @brief Unpublish a file or folder (remove public access).
     * @param disk_path Path to file or folder on Yandex.Disk.
     * @return true on success.
     * @throws std::runtime_error on API/network error.
     */
    bool unpublish(const std::string& disk_path);

    /**
     * @brief Get public download link for a published file or folder.
     * @param disk_path Path to file or folder on Yandex.Disk.
     * @return Public URL as string.
     * @throws std::runtime_error if not published or on error.
     */
    std::string getPublicDownloadLink(const std::string& disk_path);

    /**
     * @brief Upload a local file to Yandex.Disk.
     * @param disk_dir Destination directory or file path on Yandex.Disk.
     * @param local_path Path to local file.
     * @return true on success.
     * @throws std::runtime_error on API/network error.
     */
    bool uploadFile(
            const std::string& disk_dir,
            const std::string& local_path);

    /**
     * @brief Download a file from Yandex.Disk to local directory.
     * @param download_disk_path Path to file on Yandex.Disk.
     * @param local_dir Local directory to save the file.
     * @return true on success.
     * @throws std::runtime_error on API/network error.
     */
    bool downloadFile(
            const std::string& download_disk_path,
            const std::string& local_dir);

    /**
     * @brief Recursively upload a local directory to Yandex.Disk.
     * @param disk_path Destination directory on Yandex.Disk.
     * @param local_path Local directory to upload.
     * @return true on success.
     * @throws std::runtime_error on API/network error.
     */
    bool uploadDirectory(
            const std::string& disk_path,
            const std::string& local_path);

    /**
     * @brief Recursively download a directory from Yandex.Disk to local path.
     * @param disk_path Path to directory on Yandex.Disk.
     * @param local_path Local directory to save contents.
     * @return true on success.
     * @throws std::runtime_error on API/network error.
     */
    bool downloadDirectory(
            const std::string& disk_path,
            const std::string& local_path);

    /**
     * @brief Delete a file or directory from Yandex.Disk.
     * @param disk_path Path to file or directory on Yandex.Disk.
     * @return true on success.
     * @throws std::runtime_error on API/network error.
     */
    bool deleteFileOrDir(const std::string& disk_path);

    /**
     * @brief Create a directory on Yandex.Disk.
     * @param disk_path Path to directory to create.
     * @return true on success.
     * @throws std::runtime_error on API/network error.
     */
    bool createDirectory(const std::string& disk_path);

    /**
     * @brief Move or copy a file or directory on Yandex.Disk.
     * @param from_path Source path.
     * @param to_path Destination path.
     * @param overwrite Overwrite if destination exists.
     * @return true on success.
     * @throws std::runtime_error on API/network error.
     */
    bool moveFileOrDir(
            const std::string& from_path,
            const std::string& to_path,
            bool overwrite = false
    );

    /**
     * @brief Rename a file or directory on Yandex.Disk.
     * @param disk_path Path to file or directory.
     * @param new_name New name.
     * @param overwrite Overwrite if destination exists.
     * @return true on success.
     * @throws std::runtime_error on API/network error.
     */
    bool renameFileOrDir(
            const std::string& disk_path,
            const std::string& new_name,
            bool overwrite = false);

    /**
     * @brief Check if a file or directory exists on Yandex.Disk.
     * @param disk_path Path to file or directory.
     * @return true if exists, false otherwise.
     */
    bool exists(const std::string& disk_path);

    /**
     * @brief Get list of files and folders in Yandex.Disk trash.
     * @param trash_path Path in trash (default: "trash:/").
     * @return JSON object with trash resource list.
     * @throws std::runtime_error on API/network error.
     */
    nlohmann::json getTrashResourceList(const std::string& trash_path = "trash:/");

    /**
     * @brief Format trash resource list as human-readable string.
     * @param json JSON object from getTrashResourceList().
     * @return Formatted string.
     */
    std::string formatTrashResourceList(const nlohmann::json& json);

    /**
     * @brief Restore a file or directory from trash to its original location.
     * @param trash_path Path to resource in trash (from "path" field).
     * @return true on success.
     * @throws std::runtime_error on API/network error.
     */
    bool restoreFromTrash(const std::string& trash_path);

    /**
     * @brief Permanently delete a file or directory from trash.
     * @param trash_path Path to resource in trash (from "path" field).
     * @return true on success.
     * @throws std::runtime_error on API/network error.
     */
    bool deleteFromTrash(const std::string& trash_path);

    /**
     * @brief Empty the entire Yandex.Disk trash.
     * @return true on success.
     * @throws std::runtime_error on API/network error.
     */
    bool emptyTrash();

    /**
     * @brief Find all resources in trash by name.
     * @param name Name of file or folder.
     * @return Vector of full paths in trash for all matches.
     */
    std::vector<std::string> findTrashPathByName(const std::string& name);

    /**
     * @brief Find all resources on disk by name (recursive).
     * @param name Name of file or folder.
     * @param start_path Directory to start search from (default: root).
     * @return Vector of full paths for all matches.
     */
    std::vector<std::string> findResourcePathByName(
            const std::string& name,
            const std::string& start_path = "/");

private:
    std::string token;

    std::string performRequest(const std::string& url,
                               const std::string& method = "GET",
                               long* http_code = nullptr);

    std::string getUploadUrl(const std::string& upload_disk_path);

    std::string getDownloadUrl(const std::string& download_disk_path);

    std::string getLinkByKey(
            const std::string& path,
            const std::string& endpoint,
            const std::string& key,
            const std::string& extraParams,
            const std::string& errorMsg
    );

    std::string buildUrl(
            const std::string& endpoint,
            const std::map<std::string, std::string>& params
    );

    std::string buildUrl(
            const std::string& endpoint,
            const std::string& path,
            const std::string& extraParams
    );

    std::string makeUploadDiskPath(
            const std::string& upload_disk_path,
            const std::string& local_path);

    std::string makeLocalDownloadPath(
            const std::string& download_disk_path,
            const std::string& local_path);

    std::string makeDiskPath(const std::string& disk_path);

    void checkApiError(const std::string& response);

    std::vector<std::string> findPathsByName(
            const std::string& name,
            const std::string& start_path,
            std::function<nlohmann::json(const std::string&)> listFunc,
            bool recursive = true);

};


#endif //YANDEX_DISK_CPP_CLIENT_YANDEXDISKCLIENT_H
