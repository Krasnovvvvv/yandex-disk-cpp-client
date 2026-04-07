#ifndef YANDEX_DISK_CPP_CLIENT_PATHUTILS_H
#define YANDEX_DISK_CPP_CLIENT_PATHUTILS_H
#pragma once

#include <string>

namespace path_utils {
    [[nodiscard]] std::string makeDiskPath(const std::string& disk_path);

    [[nodiscard]] std::string makeUploadDiskPath(const std::string& upload_disk_path,
                                                 const std::string& local_path);

    [[nodiscard]] std::string makeLocalDownloadPath(const std::string& download_disk_path,
                                                    const std::string& local_path);
}

#endif //YANDEX_DISK_CPP_CLIENT_PATHUTILS_H
