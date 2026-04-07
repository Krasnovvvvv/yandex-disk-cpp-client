#include "PathUtils.h"
#include <filesystem>

std::string path_utils::makeDiskPath(const std::string &disk_path) {
    std::filesystem::path p(disk_path);
#if defined(_WIN32)
    return p.u8string();
#else
    return p.string();
#endif
}

std::string path_utils::makeUploadDiskPath(const std::string &upload_disk_path,
                                           const std::string &local_path) {
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

std::string path_utils::makeLocalDownloadPath(const std::string &download_disk_path,
                                              const std::string &local_path) {
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