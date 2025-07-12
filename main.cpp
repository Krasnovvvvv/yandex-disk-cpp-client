#include "YandexDiskClient.h"
#include <cstdlib>
#include <iostream>


int main() {

    const char* token_cstr = std::getenv("YADISK_TOKEN");
    std::string token(token_cstr);

    YandexDiskClient yandex(token);

    auto files = yandex.getResourceList("/");
    std::cout <<yandex.formatResourceList(files) << std::endl;
    yandex.downloadFile("/Физика/Капельян.pdf",R"(C:\Users\37529\Desktop)");
    return 0;
}

