#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "DirectoryOps.h"

namespace fs = std::filesystem;

namespace {

bool containsString(const std::vector<std::string>& values, const std::string& expected) {
    return std::find(values.begin(), values.end(), expected) != values.end();
}

} // namespace

TEST(DirectoryOpsTests, UploadDirectoryThrowsWhenLocalDirectoryDoesNotExist) {
    const fs::path missing = fs::temp_directory_path() / "ydcpp_missing_dir_for_test";

    if (fs::exists(missing)) {
        fs::remove_all(missing);
    }

    EXPECT_THROW(
        directory_ops::uploadDirectory(
            "disk:/backup",
            missing.string(),
            [](const std::string&) {},
            [](const std::string&, const std::string&) {}
        ),
        std::runtime_error
    );
}

TEST(DirectoryOpsTests, UploadDirectoryInvokesCallbacksForDirectoriesAndFiles) {
    const fs::path root = fs::temp_directory_path() / "ydcpp_upload_dir_test";
    const fs::path nested = root / "nested";
    const fs::path file1 = root / "a.txt";
    const fs::path file2 = nested / "b.txt";

    fs::remove_all(root);
    fs::create_directories(nested);

    {
        std::ofstream(file1.string()) << "alpha";
        std::ofstream(file2.string()) << "beta";
    }

    std::vector<std::string> created_dirs;
    std::vector<std::pair<std::string, std::string>> uploaded_files;

    directory_ops::uploadDirectory(
        "disk:/backup",
        root.string(),
        [&](const std::string& remote_dir) {
            created_dirs.push_back(remote_dir);
        },
        [&](const std::string& remote_path, const std::string& local_file) {
            uploaded_files.emplace_back(remote_path, local_file);
        }
    );

    EXPECT_FALSE(created_dirs.empty());
    EXPECT_EQ(uploaded_files.size(), 2u);

    fs::remove_all(root);
}

TEST(DirectoryOpsTests, DownloadDirectoryThrowsWhenRemoteListingIsNotDirectoryLike) {
    const fs::path local = fs::temp_directory_path() / "ydcpp_download_dir_test_invalid";
    fs::remove_all(local);

    auto listFunc = [](const std::string&) -> nlohmann::json {
        return nlohmann::json::object();
    };

    EXPECT_THROW(
        directory_ops::downloadDirectory(
            "disk:/docs",
            local.string(),
            listFunc,
            [](const std::string&, const std::string&) {}
        ),
        std::runtime_error
    );
}

TEST(DirectoryOpsTests, DownloadDirectoryCreatesLocalStructureAndCallsDownloadCallback) {
    const fs::path local = fs::temp_directory_path() / "ydcpp_download_dir_test";
    fs::remove_all(local);

    auto listFunc = [](const std::string& path) -> nlohmann::json {
        if (path == "disk:/docs") {
            return nlohmann::json{
                {"_embedded", {
                    {"items", {
                        {{"name", "nested"}, {"type", "dir"}, {"path", "disk:/docs/nested"}},
                        {{"name", "root.txt"}, {"type", "file"}, {"path", "disk:/docs/root.txt"}}
                    }}
                }}
            };
        }

        if (path == "disk:/docs/nested") {
            return nlohmann::json{
                {"_embedded", {
                    {"items", {
                        {{"name", "inner.txt"}, {"type", "file"}, {"path", "disk:/docs/nested/inner.txt"}}
                    }}
                }}
            };
        }

        return nlohmann::json{
            {"_embedded", {{"items", nlohmann::json::array()}}}
        };
    };

    std::vector<std::pair<std::string, std::string>> downloaded_files;

    directory_ops::downloadDirectory(
        "disk:/docs",
        local.string(),
        listFunc,
        [&](const std::string& remote_file, const std::string& local_dir) {
            downloaded_files.emplace_back(remote_file, local_dir);
            fs::create_directories(local_dir);
            std::ofstream(fs::path(local_dir) / fs::path(remote_file).filename()) << "stub";
        }
    );

    EXPECT_EQ(downloaded_files.size(), 2u);
    EXPECT_TRUE(fs::exists(local));
    EXPECT_TRUE(fs::exists(local / "root.txt"));
    EXPECT_TRUE(fs::exists(local / "nested" / "inner.txt"));

    fs::remove_all(local);
}

TEST(DirectoryOpsTests, DownloadDirectoryThrowsWhenLocalPathExistsAsFile) {
    const fs::path local_file = fs::temp_directory_path() / "ydcpp_download_conflict.txt";
    {
        std::ofstream(local_file.string()) << "conflict";
    }

    auto listFunc = [](const std::string&) -> nlohmann::json {
        return nlohmann::json{
            {"_embedded", {{"items", nlohmann::json::array()}}}
        };
    };

    EXPECT_THROW(
        directory_ops::downloadDirectory(
            "disk:/docs",
            local_file.string(),
            listFunc,
            [](const std::string&, const std::string&) {}
        ),
        std::runtime_error
    );

    fs::remove(local_file);
}