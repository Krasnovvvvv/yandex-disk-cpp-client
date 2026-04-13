#include "QueryOps.h"
#include "APIUtils.h"

#include <map>
#include <string>
#include <stdexcept>

namespace {

std::string requestBody(HttpClient& client,
                        const std::string& endpoint,
                        const std::string& path,
                        const std::string& extra_params = "") {
    const std::string url = api_utils::buildUrl(
        endpoint,
        path,
        extra_params,
        client
    );

    auto response = client.request(url, "GET");
    api_utils::checkApiError(response.body);
    return response.body;
}

std::string requestBody(HttpClient& client,
                        const std::string& endpoint,
                        const std::map<std::string, std::string>& params) {
    const std::string url = api_utils::buildUrl(
        endpoint,
        params,
        client
    );

    auto response = client.request(url, "GET");
    api_utils::checkApiError(response.body);
    return response.body;
}

nlohmann::json requestJson(HttpClient& client,
                           const std::string& endpoint,
                           const std::string& path,
                           const std::string& extra_params = "") {
    return nlohmann::json::parse(
        requestBody(client, endpoint, path, extra_params)
    );
}

nlohmann::json requestJson(HttpClient& client,
                           const std::string& endpoint,
                           const std::map<std::string, std::string>& params) {
    return nlohmann::json::parse(
        requestBody(client, endpoint, params)
    );
}

std::string requestLink(HttpClient& client,
                        const std::string& endpoint,
                        const std::string& path,
                        const std::string& key,
                        const std::string& error_message,
                        const std::string& extra_params = "") {
    return api_utils::extractLinkByKey(
        requestBody(client, endpoint, path, extra_params),
        key,
        error_message
    );
}

} // namespace

nlohmann::json query_ops::getQuotaInfo(HttpClient& client) {
    return requestJson(
        client,
        "https://cloud-api.yandex.net/v1/disk",
        std::map<std::string, std::string>{}
    );
}

nlohmann::json query_ops::getResourceList(HttpClient& client,
                                          const std::string& disk_path) {
    return requestJson(
        client,
        "https://cloud-api.yandex.net/v1/disk/resources?path=",
        disk_path
    );
}

nlohmann::json query_ops::getResourceInfo(HttpClient& client,
                                          const std::string& disk_path) {
    return requestJson(
        client,
        "https://cloud-api.yandex.net/v1/disk/resources",
        {
            {"path", disk_path}
        }
    );
}

nlohmann::json query_ops::getTrashResourceList(HttpClient& client,
                                               const std::string& trash_path) {
    return requestJson(
        client,
        "https://cloud-api.yandex.net/v1/disk/trash/resources",
        {
            {"path", trash_path}
        }
    );
}

std::string query_ops::getPublicDownloadLink(HttpClient& client,
                                             const std::string& disk_path) {
    return requestLink(
        client,
        "https://cloud-api.yandex.net/v1/disk/resources?path=",
        disk_path,
        "public_url",
        "The file or directory has not been published!"
            " Use publish() method to publish it."
    );
}

std::string query_ops::getUploadUrl(HttpClient& client,
                                    const std::string& upload_disk_path) {
    return requestLink(
        client,
        "https://cloud-api.yandex.net/v1/disk/resources/upload?path=",
        upload_disk_path,
        "href",
        "Upload URL not found in API response.",
        "&overwrite=true"
    );
}

std::string query_ops::getDownloadUrl(HttpClient& client,
                                      const std::string& download_disk_path) {
    return requestLink(
        client,
        "https://cloud-api.yandex.net/v1/disk/resources/download?path=",
        download_disk_path,
        "href",
        "Download URL not found in API response."
    );
}

bool query_ops::exists(HttpClient& client,
                       const std::string& disk_path) {
    try {
        const std::string url = api_utils::buildUrl(
            "https://cloud-api.yandex.net/v1/disk/resources",
            {
                {"path", disk_path}
            },
            client
        );

        auto response = client.request(url, "GET");
        return response.status_code == 200;
    } catch (const std::exception&) {
        return false;
    }
}