#include "YandexDiskClient.h"
#include <cstdlib>
#include <iostream>
#include <windows.h>

int main() {

    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
    const char* token_cstr = std::getenv("YADISK_TOKEN");
    std::string token(token_cstr);

    YandexDiskClient yandex(token);

    auto files = yandex.getResourceList("/");
    std::cout <<yandex.formatResourceList(files) << std::endl;
    return 0;
}

