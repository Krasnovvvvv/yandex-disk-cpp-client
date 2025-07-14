// Example: Recursive upload and download of directories
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
        // Upload a local directory to Yandex.Disk
        yandex.uploadDirectory("/backup_folder", "C:/local/backup_folder");

        // Download a directory from Yandex.Disk
        yandex.downloadDirectory("/backup_folder", "C:/local/restore_folder");

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    return 0;
}

