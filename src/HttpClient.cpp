#include "HttpClient.h"

#include <stdexcept>
#include <curl/curl.h>
#include <filesystem>

HttpClient::HttpClient(const std::string& oauth_token)
    : token_(oauth_token) {}

HttpResponse HttpClient::request(const std::string &url, const std::string &method) const {
    CURL* curl = curl_easy_init();
    std::string response;
    if (!curl) throw std::runtime_error("curl_easy_init() failed");

    curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: OAuth " + token_).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    if (method == "PUT") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    } else if (method == "DELETE") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
    } else if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
    }

    CURLcode res = curl_easy_perform(curl);

    long code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) throw std::runtime_error(curl_easy_strerror(res));
    return {code, response};
}

void HttpClient::uploadFileByUrl(const std::string &url, const std::string &local_path) const {
#if defined(_WIN32)
    FILE* file = _wfopen(std::filesystem::u8path(local_path).wstring().c_str(), L"rb");
#else
    FILE* file = fopen(local_path.c_str(), "rb");
#endif

    if (!file) {
        throw std::runtime_error("Couldn't open the file: " + local_path);
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        fclose(file);
        throw std::runtime_error("curl_easy_init() failed");
    }

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    if (filesize < 0) {
        fclose(file);
        curl_easy_cleanup(curl);
        throw std::runtime_error("Failed to determine file size: " + local_path);
    }
    fseek(file, 0, SEEK_SET);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_READDATA, file);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(filesize));

    CURLcode res = curl_easy_perform(curl);

    fclose(file);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        throw std::runtime_error("File upload error: " +
                                 std::string(curl_easy_strerror(res)));
    }
}

void HttpClient::downloadToFile(const std::string &url, const std::string &local_path) const {
#if defined(_WIN32)
    FILE* file = _wfopen(std::filesystem::u8path(local_path).wstring().c_str(), L"wb");
#else
    FILE* file = fopen(local_path.c_str(), "wb");
#endif
    if (!file) {
        throw std::runtime_error("Failed to create a file: " + local_path);
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        fclose(file);
        throw std::runtime_error("curl_easy_init() failed");
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, nullptr);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);

    fclose(file);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        throw std::runtime_error("File download error: " +
                                 std::string(curl_easy_strerror(res)));
    }
}

std::string HttpClient::urlEncode(const std::string& value) const {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("curl_easy_init() failed");
    }

    char* escaped = curl_easy_escape(curl, value.c_str(), 0);
    if (!escaped) {
        curl_easy_cleanup(curl);
        throw std::runtime_error("curl_easy_escape() failed");
    }

    std::string encoded = escaped;
    curl_free(escaped);
    curl_easy_cleanup(curl);

    return encoded;
}
