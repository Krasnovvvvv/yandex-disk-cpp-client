// Example: Search for files/folders by name
#include <iostream>
#include <cstdlib>
#include <vector>
#include "YandexDiskClient.h"

int main() {
    const char* token = std::getenv("YADISK_TOKEN");
    if (!token) {
        std::cerr << "Please set the YADISK_TOKEN environment variable." << std::endl;
        return 1;
    }

    YandexDiskClient yandex(token);

    try {
        std::string fileName = "data.zip";

        // Search on disk
        auto results = yandex.findResourcePathByName(fileName);
        std::cout << "Found on disk:" << std::endl;
        for (const auto& path : results) {
            std::cout << "  " << path << std::endl;
        }

        // Search in trash
        auto trashResults = yandex.findTrashPathByName(fileName);
        std::cout << "Found in trash:" << std::endl;
        for (const auto& path : trashResults) {
            std::cout << "  " << path << std::endl;
        }

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    return 0;
}
