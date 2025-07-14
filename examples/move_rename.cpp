// Example: Move and rename files/directories
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
        std::string fromPath = "/backup/old_name.zip";
        std::string toPath = "/backup/new_name.zip";

        // Move or rename a file or folder
        if (yandex.moveFileOrDir(fromPath, toPath, true)) {
            std::cout << "Moved/Renamed from " << fromPath << " to " << toPath << std::endl;
        }

        // Rename a file
        std::string renamePath = "/backup/new_name.zip";
        std::string newName = "renamed_file.zip";
        if (yandex.renameFileOrDir(renamePath, newName, true)) {
            std::cout << "Renamed " << renamePath << " to " << newName << std::endl;
        }

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    return 0;
}

