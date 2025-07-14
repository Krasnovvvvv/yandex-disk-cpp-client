// Example: Publishing and unpublishing files/folders
#include <iostream>
#include <cstdlib>
#include "YandexDiskClient.h"

int main() {
    const char* token = std::getenv("YADISK_TOKEN");
    if (!token) {
        std::cerr << "Please set the YADISK_TOKEN environment variable." << std::endl;
        return 1;
    }

    YandexDiskClient yandex(token);

    try {
        std::string path = "/backup/data.zip";

        // Publish the file (make it publicly accessible)
        if (yandex.publish(path)) {
            std::cout << "Published: " << path << std::endl;
            std::cout << "Public URL: " << yandex.getPublicDownloadLink(path) << std::endl;
        }

        // Unpublish the file
        if (yandex.unpublish(path)) {
            std::cout << "Unpublished: " << path << std::endl;
        }

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    return 0;
}

