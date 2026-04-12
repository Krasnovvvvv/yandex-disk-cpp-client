#ifndef YANDEX_DISK_CPP_CLIENT_DIRECTORYOPS_H
#define YANDEX_DISK_CPP_CLIENT_DIRECTORYOPS_H
#pragma once

#include <nlohmann/json.hpp>
#include <functional>
#include <string>

namespace directory_ops {
    using CreateDirectoryFn = std::function<void (const std::string&)>;
    using UploadFileFn = std::function<void (const std::string&, const std::string&)>;
    using ListDirectoryFn = std::function<nlohmann::json (const std::string&)>;
    using DownloadFileFn = std::function<void (const std::string&, const std::string&)>;

    void uploadDirectory(const std::string& disk_path,
                         const std::string& local_path,
                         const CreateDirectoryFn& create_directory,
                         const UploadFileFn& upload_file);

    void downloadDirectory(const std::string& disk_path,
                           const std::string& local_path,
                           const ListDirectoryFn& list_directory,
                           const DownloadFileFn& download_file);

} // namespace directory_ops
#endif //YANDEX_DISK_CPP_CLIENT_DIRECTORYOPS_H
