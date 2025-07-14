cmake_minimum_required(VERSION 3.28)
project(yandex-disk-cpp-client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)

file(GLOB SOURCES "src/*.cpp")

add_library(yandex_disk_client STATIC ${SOURCES})

target_include_directories(yandex_disk_client
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(yandex_disk_client
        PUBLIC nlohmann_json::nlohmann_json
        CURL::libcurl
)

add_executable(yandex_disk_example examples/main.cpp)

target_link_libraries(yandex_disk_example
        PRIVATE yandex_disk_client
)
