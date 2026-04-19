#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include "PathUtils.h"

namespace fs = std::filesystem;

TEST(PathUtilsTests, MakeDiskPathReturnsNonEmptyPathForSimpleInput) {
    const std::string input = "docs/report.txt";
    const std::string result = path_utils::makeDiskPath(input);

    EXPECT_FALSE(result.empty());
}

TEST(PathUtilsTests, MakeUploadDiskPathAppendsLocalFilenameWhenDiskPathIsDirectory) {
    const std::string disk_dir = "remote/folder";
    const std::string local_path =
#ifdef _WIN32
        R"(C:\tmp\photo.png)";
#else
        "/tmp/photo.png";
#endif

    const std::string result = path_utils::makeUploadDiskPath(disk_dir, local_path);
    const fs::path result_path(result);

    EXPECT_EQ(result_path.filename(), fs::path(local_path).filename());
}

TEST(PathUtilsTests, MakeUploadDiskPathKeepsExplicitRemoteFilename) {
    const std::string disk_file = "remote/folder/final-name.png";
    const std::string local_path =
#ifdef _WIN32
        R"(C:\tmp\photo.png)";
#else
        "/tmp/photo.png";
#endif

    const std::string result = path_utils::makeUploadDiskPath(disk_file, local_path);
    const fs::path result_path(result);

    EXPECT_EQ(result_path.filename(), fs::path("final-name.png"));
}

TEST(PathUtilsTests, MakeLocalDownloadPathAppendsRemoteFilenameWhenLocalPathIsDirectory) {
    const std::string remote_file = "disk:/docs/archive.zip";
    const std::string local_dir =
#ifdef _WIN32
        R"(C:\downloads)";
#else
        "/downloads";
#endif

    const std::string result = path_utils::makeLocalDownloadPath(remote_file, local_dir);
    const fs::path result_path(result);

    EXPECT_EQ(result_path.filename(), fs::path(remote_file).filename());
}

TEST(PathUtilsTests, MakeLocalDownloadPathKeepsExplicitLocalFilename) {
    const std::string remote_file = "disk:/docs/archive.zip";
    const std::string local_file =
#ifdef _WIN32
        R"(C:\downloads\custom-name.zip)";
#else
        "/downloads/custom-name.zip";
#endif

    const std::string result = path_utils::makeLocalDownloadPath(remote_file, local_file);
    const fs::path result_path(result);

    EXPECT_EQ(result_path.filename(), fs::path(local_file).filename());
}

TEST(PathUtilsTests, ResolveMoveDestinationUsesSourceParentWhenTargetHasNoParent) {
    const fs::path from = fs::path("folder/old_name.txt");
    const fs::path to_input = fs::path("new_name.txt");

    const fs::path resolved =
        path_utils::resolveMoveDestination(from, to_input, "new_name.txt");

    EXPECT_EQ(resolved.parent_path(), from.parent_path());
    EXPECT_EQ(resolved.filename(), fs::path("new_name.txt"));
}

TEST(PathUtilsTests, ResolveMoveDestinationAppendsSourceFilenameWhenTargetEndsWithSlash) {
    const fs::path from = fs::path("folder/old_name.txt");
    const fs::path to_input = fs::path("archive/subdir");

    const fs::path resolved =
        path_utils::resolveMoveDestination(from, to_input, "archive/subdir/");

    EXPECT_EQ(resolved.parent_path(), fs::path("archive/subdir"));
    EXPECT_EQ(resolved.filename(), from.filename());
}

TEST(PathUtilsTests, ResolveMoveDestinationKeepsExplicitTargetFilename) {
    const fs::path from = fs::path("folder/old_name.txt");
    const fs::path to_input = fs::path("archive/new_name.txt");

    const fs::path resolved =
        path_utils::resolveMoveDestination(from, to_input, "archive/new_name.txt");

    EXPECT_EQ(resolved, fs::path("archive/new_name.txt"));
}