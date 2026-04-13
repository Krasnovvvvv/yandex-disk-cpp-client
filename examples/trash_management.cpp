// Example: Trash operations (list, restore, empty)
#include <iostream>
#include <cstdlib>
#include "YandexDiskClient.h"
#include "FormatUtils.h"

int main() {
    const char* token = std::getenv("YADISK_TOKEN");
    if (!token) {
        std::cerr << "Please set the YADISK_TOKEN environment variable." << std::endl;
        return 1;
    }

    YandexDiskClient yandex(token);

    try {
        // Get list of trash contents
        auto trashList = yandex.getTrashResourceList();
        std::cout << format_utils::formatTrashResourceList(trashList) << std::endl;

        // Restore the first item from trash (if any)
        if (!trashList["_embedded"]["items"].empty()) {
            std::string trashPath = trashList["_embedded"]["items"][0].value("path", "");
            if (!trashPath.empty()) {
                yandex.restoreFromTrash(trashPath);
                std::cout << "Restored from trash: " << trashPath << std::endl;
            }
        }

        // Uncomment to empty the entire trash
        // yandex.emptyTrash();

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
}

