#ifndef YANDEX_DISK_CPP_CLIENT_PATHUTILS_H
#define YANDEX_DISK_CPP_CLIENT_PATHUTILS_H
#pragma once

#include <string>
#include <filesystem>

namespace path_utils {
    [[nodiscard]] std::string makeDiskPath(const std::string& disk_path);

    [[nodiscard]] std::string makeUploadDiskPath(const std::string& upload_disk_path,
                                                 const std::string& local_path);

    [[nodiscard]] std::string makeLocalDownloadPath(const std::string& download_disk_path,
                                                    const std::string& local_path);

    [[nodiscard]] std::filesystem::path resolveMoveDestination(
        const std::filesystem::path& from_fs,
        const std::filesystem::path& to_fs_input,
        const std::string& to_path_raw
    );
}

#endif //YANDEX_DISK_CPP_CLIENT_PATHUTILS_H
