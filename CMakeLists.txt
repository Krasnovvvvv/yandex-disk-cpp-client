cmake_minimum_required(VERSION 3.28)
project(yandex-disk-cpp-client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)

file(GLOB SOURCES "src/*.cpp")

add_library(yandex_disk_client STATIC ${SOURCES})

target_include_directories(yandex_disk_client
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)


target_link_libraries(yandex_disk_client
        PUBLIC nlohmann_json::nlohmann_json
        CURL::libcurl
)

# === Build each example as a separate executable ===

add_executable(example_basic_usage examples/basic_usage.cpp)
target_link_libraries(example_basic_usage PRIVATE yandex_disk_client)

add_executable(example_trash_management examples/trash_management.cpp)
target_link_libraries(example_trash_management PRIVATE yandex_disk_client)

add_executable(example_publish_unpublish examples/publish_unpublish.cpp)
target_link_libraries(example_publish_unpublish PRIVATE yandex_disk_client)

add_executable(example_move_rename examples/move_rename.cpp)
target_link_libraries(example_move_rename PRIVATE yandex_disk_client)

add_executable(example_search_files examples/search_files.cpp)
target_link_libraries(example_search_files PRIVATE yandex_disk_client)

add_executable(example_directory_upload_download examples/directory_upload_download.cpp)
target_link_libraries(example_directory_upload_download PRIVATE yandex_disk_client)

# === Installing a static library ===

install(TARGETS yandex_disk_client
        EXPORT yandex_disk_client_targets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
)

# === Copying all public headlines ===

install(DIRECTORY include/ DESTINATION include)

# === Exporting targets for find_package() ===

install(EXPORT yandex_disk_client_targets
        FILE yandex_disk_clientTargets.cmake
        NAMESPACE yandex_disk_client::
        DESTINATION lib/cmake/yandex_disk_client
)


