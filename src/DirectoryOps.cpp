#include "DirectoryOps.h"

#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

namespace {

std::string toRemotePathString(const fs::path& path) {
#if defined(_WIN32)
    return path.generic_u8string();
#else
    return path.generic_string();
#endif
}

std::string toLocalPathString(const fs::path& path) {
#if defined(_WIN32)
    return path.u8string();
#else
    return path.string();
#endif
}

fs::path resolveUploadRoot(const std::string& disk_path, const fs::path& local_root) {
    fs::path remote_root(disk_path);

    if (remote_root.empty() ||
        !remote_root.has_filename() ||
        (!disk_path.empty() && (disk_path.back() == '/' || disk_path.back() == '\\'))) {
        remote_root /= local_root.filename();
    }

    return remote_root;
}

fs::path resolveInitialDownloadRoot(const std::string& disk_path, const std::string& local_path) {
    fs::path remote_root(disk_path);
    fs::path local_root(local_path);

    fs::path folder_name = remote_root.filename();
    if (folder_name.empty()) {
        folder_name = remote_root.parent_path().filename();
    }
    if (folder_name.empty()) {
        folder_name = "download";
    }

    if (!fs::exists(local_root)) {
        return local_root;
    }

    if (!fs::is_directory(local_root)) {
        throw std::runtime_error("Local path exists and is not a directory: " + local_path);
    }

    return local_root / folder_name;
}

void downloadDirectoryImpl(const std::string& remote_dir_path,
                           const fs::path& local_dir_path,
                           const directory_ops::ListDirectoryFn& list_directory,
                           const directory_ops::DownloadFileFn& download_file) {
    auto info = list_directory(remote_dir_path);

    if (!info.contains("_embedded") || !info["_embedded"].contains("items")) {
        throw std::runtime_error("Remote directory does not exist or is not a directory: " + remote_dir_path);
    }

    fs::create_directories(local_dir_path);

    for (const auto& item : info["_embedded"]["items"]) {
        const std::string name = item.value("name", "");
        const std::string type = item.value("type", "");
        const std::string remote_item_path = item.value("path", "");

        fs::path local_item_path = local_dir_path / name;

        if (type == "dir") {
            downloadDirectoryImpl(remote_item_path, local_item_path, list_directory, download_file);
        } else if (type == "file") {
            download_file(remote_item_path, toLocalPathString(local_dir_path));
        }
    }
}

} // namespace

void directory_ops::uploadDirectory(const std::string& disk_path,
                                    const std::string& local_path,
                                    const CreateDirectoryFn& create_directory,
                                    const UploadFileFn& upload_file) {
    fs::path local_root(local_path);

    if (!fs::exists(local_root) || !fs::is_directory(local_root)) {
        throw std::runtime_error("Local directory does not exist: " + local_path);
    }

    fs::path remote_root = resolveUploadRoot(disk_path, local_root);
    create_directory(toRemotePathString(remote_root));

    for (const auto& entry : fs::recursive_directory_iterator(local_root)) {
        fs::path relpath = fs::relative(entry.path(), local_root);
        fs::path remote_target = remote_root / relpath;

        if (entry.is_directory()) {
            create_directory(toRemotePathString(remote_target));
        } else if (entry.is_regular_file()) {
            upload_file(toRemotePathString(remote_target),
                        toLocalPathString(entry.path()));
        }
    }
}

void directory_ops::downloadDirectory(const std::string& disk_path,
                                      const std::string& local_path,
                                      const ListDirectoryFn& list_directory,
                                      const DownloadFileFn& download_file) {
    fs::path local_root = resolveInitialDownloadRoot(disk_path, local_path);
    downloadDirectoryImpl(disk_path, local_root, list_directory, download_file);
}