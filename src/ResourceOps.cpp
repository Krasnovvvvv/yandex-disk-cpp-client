#include "ResourceOps.h"
#include "APIUtils.h"
#include "HttpClient.h"

namespace {

    bool performRequest(HttpClient& client,
                        const std::string& method,
                        const std::string& endpoint,
                        const std::string& path,
                        const std::string& extra_params = "") {
        const std::string url = api_utils::buildUrl(
            endpoint,
            path,
            extra_params,
            client
        );

        auto response = client.request(url, method);
        api_utils::checkApiError(response.body);

        return true;
    }

    bool performRequest(HttpClient& client,
                        const std::string& method,
                        const std::string& endpoint,
                        const std::map<std::string, std::string>& params) {
        const std::string url = api_utils::buildUrl(
            endpoint,
            params,
            client
        );

        auto response = client.request(url, method);
        api_utils::checkApiError(response.body);

        return true;
    }

} // namespace

bool resource_ops::createDirectory(HttpClient& client, const std::string& disk_path) {
    return performRequest(
        client,
        "PUT",
        "https://cloud-api.yandex.net/v1/disk/resources?path=",
        disk_path
    );
}

bool resource_ops::deleteFileOrDir(HttpClient& client, const std::string& disk_path) {
    return performRequest(
        client,
        "DELETE",
        "https://cloud-api.yandex.net/v1/disk/resources?path=",
        disk_path
    );
}

bool resource_ops::moveFileOrDir(HttpClient& client,
                                 const std::string& from_path,
                                 const std::string& to_path,
                                 bool overwrite) {
    std::map<std::string, std::string> params{
        {"from", from_path},
        {"path", to_path}
    };
    if (overwrite) {
        params["overwrite"] = "true";
    }

    return performRequest(
        client,
        "POST",
        "https://cloud-api.yandex.net/v1/disk/resources/move",
        params
    );
}

bool resource_ops::publish(HttpClient& client, const std::string& disk_path) {
    return performRequest(
        client,
        "PUT",
        "https://cloud-api.yandex.net/v1/disk/resources/publish?path=",
        disk_path
    );
}

bool resource_ops::unpublish(HttpClient& client, const std::string& disk_path) {
    return performRequest(
        client,
        "PUT",
        "https://cloud-api.yandex.net/v1/disk/resources/unpublish",
        {
            {"path", disk_path}
        }
    );
}

bool resource_ops::restoreFromTrash(HttpClient& client, const std::string& trash_path) {
    return performRequest(
        client,
        "PUT",
        "https://cloud-api.yandex.net/v1/disk/trash/resources/restore",
        {
            {"path", trash_path}
        }
    );
}

bool resource_ops::deleteFromTrash(HttpClient& client, const std::string& trash_path) {
    return performRequest(
        client,
        "DELETE",
        "https://cloud-api.yandex.net/v1/disk/trash/resources",
        {
            {"path", trash_path}
        }
    );
}

bool resource_ops::emptyTrash(HttpClient& client) {
    return performRequest(
        client,
        "DELETE",
        "https://cloud-api.yandex.net/v1/disk/trash/resources?path=",
        ""
    );
}