cmake_minimum_required(VERSION 3.28)
project(yandex-disk-cpp-client)
set(CMAKE_CXX_STANDARD 17)

add_executable(yandex-disk-cpp-client
        main.cpp
        YandexDiskClient.cpp
        YandexDiskClient.h)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)
target_link_libraries(yandex-disk-cpp-client PRIVATE
                      nlohmann_json::nlohmann_json
                      CURL::libcurl)