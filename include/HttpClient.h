#ifndef YANDEX_DISK_CPP_CLIENT_HTTPCLIENT_H
#define YANDEX_DISK_CPP_CLIENT_HTTPCLIENT_H
#pragma once

#include <string>

struct HttpResponse {
    long status_code = 0;
    std::string body;
};

class HttpClient {
public:
    explicit HttpClient(const std::string& oauth_token);

    HttpResponse request(const std::string& url,
                         const std::string& method = "GET") const;
private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        static_cast<std::string *>(userp)->append(static_cast<char *>(contents), size * nmemb);
        return size * nmemb;
    }

    std::string token_;
};
#endif //YANDEX_DISK_CPP_CLIENT_HTTPCLIENT_H
