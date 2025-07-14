#include "YandexDiskClient.h"
#include <cstdlib>
#include <iostream>

int main() {
    // Get OAuth token from environment variable
    const char* token = std::getenv("YADISK_TOKEN");
    if (!token) {
        std::cerr << "Please set YADISK_TOKEN environment variable." << std::endl;
        return 1;
    }

    YandexDiskClient yandex(token);

    try {
        // Upload a file
        yandex.uploadFile("/backup/data.zip", "C:/local/data.zip");

        // Download a file
        yandex.downloadFile("/backup/data.zip", "C:/local/restore.zip");

        // List root directory contents
        auto list = yandex.getResourceList("/");
        std::cout << yandex.formatResourceList(list) << std::endl;

        // Get quota info
        auto quota = yandex.getQuotaInfo();
        std::cout << yandex.formatQuotaInfo(quota) << std::endl;

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    return 0;
}


