#include <gtest/gtest.h>

#include <string>
#include <nlohmann/json.hpp>

#include "FormatUtils.h"

TEST(FormatUtilsTests, FormatQuotaInfoContainsMainFields) {
    const nlohmann::json quota = {
        {"total_space", 1073741824},
        {"used_space", 536870912},
        {"trash_size", 1024}
    };

    const std::string result = format_utils::formatQuotaInfo(quota);

    EXPECT_NE(result.find("Total space"), std::string::npos);
    EXPECT_NE(result.find("Used"), std::string::npos);
    EXPECT_NE(result.find("In trash"), std::string::npos);
}

TEST(FormatUtilsTests, FormatResourceInfoContainsMainMetadata) {
    const nlohmann::json info = {
        {"name", "report.pdf"},
        {"path", "disk:/docs/report.pdf"},
        {"type", "file"},
        {"size", 2048},
        {"created", "2026-04-14T12:00:00+00:00"},
        {"modified", "2026-04-14T12:10:00+00:00"},
        {"public_url", "https://example.test/public"},
        {"md5", "abc123"}
    };

    const std::string result = format_utils::formatResourceInfo(info);

    EXPECT_NE(result.find("report.pdf"), std::string::npos);
    EXPECT_NE(result.find("disk:/docs/report.pdf"), std::string::npos);
    EXPECT_NE(result.find("file"), std::string::npos);
    EXPECT_NE(result.find("https://example.test/public"), std::string::npos);
    EXPECT_NE(result.find("abc123"), std::string::npos);
}

TEST(FormatUtilsTests, FormatResourceInfoHandlesDirectoryWithoutSize) {
    const nlohmann::json info = {
        {"name", "docs"},
        {"path", "disk:/docs"},
        {"type", "dir"},
        {"created", "2026-04-14T12:00:00+00:00"},
        {"modified", "2026-04-14T12:10:00+00:00"},
        {"public_url", nullptr},
        {"md5", ""}
    };

    const std::string result = format_utils::formatResourceInfo(info);

    EXPECT_NE(result.find("docs"), std::string::npos);
    EXPECT_NE(result.find("disk:/docs"), std::string::npos);
    EXPECT_NE(result.find("dir"), std::string::npos);
}

TEST(FormatUtilsTests, FormatResourceListFormatsEmbeddedItems) {
    const nlohmann::json list = {
        {"_embedded", {
            {"items", {
                {
                    {"name", "photo.jpg"},
                    {"type", "file"},
                    {"path", "disk:/images/photo.jpg"},
                    {"public_url", "https://example.test/photo"}
                },
                {
                    {"name", "docs"},
                    {"type", "dir"},
                    {"path", "disk:/docs"}
                }
            }}
        }}
    };

    const std::string result = format_utils::formatResourceList(list);

    EXPECT_NE(result.find("photo.jpg"), std::string::npos);
    EXPECT_NE(result.find("disk:/images/photo.jpg"), std::string::npos);
    EXPECT_NE(result.find("docs"), std::string::npos);
}

TEST(FormatUtilsTests, FormatTrashResourceListFormatsTrashItems) {
    const nlohmann::json list = {
        {"_embedded", {
            {"items", {
                {
                    {"name", "old.txt"},
                    {"type", "file"},
                    {"path", "trash:/old.txt"},
                    {"origin_path", "disk:/docs/old.txt"},
                    {"created", "2026-04-14T12:00:00+00:00"},
                    {"deleted", "2026-04-14T12:30:00+00:00"},
                    {"size", 512}
                }
            }}
        }}
    };

    const std::string result = format_utils::formatTrashResourceList(list);

    EXPECT_NE(result.find("old.txt"), std::string::npos);
    EXPECT_NE(result.find("trash:/old.txt"), std::string::npos);
    EXPECT_NE(result.find("disk:/docs/old.txt"), std::string::npos);
}

TEST(FormatUtilsTests, FormatTrashResourceListHandlesEmptyTrash) {
    const nlohmann::json list = {
        {"_embedded", {
            {"items", nlohmann::json::array()}
        }}
    };

    const std::string result = format_utils::formatTrashResourceList(list);

    EXPECT_NE(result.find("Trash is empty"), std::string::npos);
}