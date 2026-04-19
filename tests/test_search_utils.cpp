#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "SearchUtils.h"

namespace {

nlohmann::json makeDirListing(const std::vector<nlohmann::json>& items) {
    return nlohmann::json{
        {"_embedded", {
            {"items", items}
        }}
    };
}

bool containsPath(const std::vector<std::string>& paths, const std::string& expected) {
    return std::find(paths.begin(), paths.end(), expected) != paths.end();
}

} // namespace

TEST(SearchUtilsTests, FindPathsByNameFindsTopLevelMatch) {
    auto listFunc = [](const std::string& path) -> nlohmann::json {
        if (path == "/") {
            return makeDirListing({
                {{"name", "report.txt"}, {"type", "file"}, {"path", "disk:/report.txt"}},
                {{"name", "docs"}, {"type", "dir"}, {"path", "disk:/docs"}}
            });
        }
        return makeDirListing({});
    };

    const auto result = search_utils::findPathsByName("report.txt", "/", listFunc, true);

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result.front(), "disk:/report.txt");
}

TEST(SearchUtilsTests, FindPathsByNameFindsNestedMatchRecursively) {
    auto listFunc = [](const std::string& path) -> nlohmann::json {
        if (path == "/") {
            return makeDirListing({
                {{"name", "docs"}, {"type", "dir"}, {"path", "disk:/docs"}}
            });
        }
        if (path == "disk:/docs") {
            return makeDirListing({
                {{"name", "report.txt"}, {"type", "file"}, {"path", "disk:/docs/report.txt"}}
            });
        }
        return makeDirListing({});
    };

    const auto result = search_utils::findPathsByName("report.txt", "/", listFunc, true);

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result.front(), "disk:/docs/report.txt");
}

TEST(SearchUtilsTests, FindPathsByNameDoesNotDescendWhenRecursiveDisabled) {
    auto listFunc = [](const std::string& path) -> nlohmann::json {
        if (path == "/") {
            return makeDirListing({
                {{"name", "docs"}, {"type", "dir"}, {"path", "disk:/docs"}}
            });
        }
        if (path == "disk:/docs") {
            return makeDirListing({
                {{"name", "report.txt"}, {"type", "file"}, {"path", "disk:/docs/report.txt"}}
            });
        }
        return makeDirListing({});
    };

    const auto result = search_utils::findPathsByName("report.txt", "/", listFunc, false);

    EXPECT_TRUE(result.empty());
}

TEST(SearchUtilsTests, FindPathsByNameReturnsAllMatches) {
    auto listFunc = [](const std::string& path) -> nlohmann::json {
        if (path == "/") {
            return makeDirListing({
                {{"name", "report.txt"}, {"type", "file"}, {"path", "disk:/report.txt"}},
                {{"name", "docs"}, {"type", "dir"}, {"path", "disk:/docs"}}
            });
        }
        if (path == "disk:/docs") {
            return makeDirListing({
                {{"name", "report.txt"}, {"type", "file"}, {"path", "disk:/docs/report.txt"}}
            });
        }
        return makeDirListing({});
    };

    const auto result = search_utils::findPathsByName("report.txt", "/", listFunc, true);

    ASSERT_EQ(result.size(), 2u);
    EXPECT_TRUE(containsPath(result, "disk:/report.txt"));
    EXPECT_TRUE(containsPath(result, "disk:/docs/report.txt"));
}

TEST(SearchUtilsTests, FindPathsByNameHandlesMissingEmbeddedGracefully) {
    auto listFunc = [](const std::string&) -> nlohmann::json {
        return nlohmann::json::object();
    };

    const auto result = search_utils::findPathsByName("report.txt", "/", listFunc, true);

    EXPECT_TRUE(result.empty());
}