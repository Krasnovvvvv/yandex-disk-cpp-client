cmake_minimum_required(VERSION 3.28)
project(yandex-disk-cpp-client VERSION 1.0.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_EXAMPLES "Build example executables" ON)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)

file(GLOB SOURCES "src/*.cpp")

add_library(yandex-disk-cpp-client STATIC ${SOURCES})

target_include_directories(yandex-disk-cpp-client
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(yandex-disk-cpp-client
        PUBLIC nlohmann_json::nlohmann_json
        CURL::libcurl
)

# === Build each example as a separate executable ===
if(BUILD_EXAMPLES)
    add_executable(example_basic_usage examples/basic_usage.cpp)
    target_link_libraries(example_basic_usage PRIVATE yandex-disk-cpp-client)

    add_executable(example_trash_management examples/trash_management.cpp)
    target_link_libraries(example_trash_management PRIVATE yandex-disk-cpp-client)

    add_executable(example_publish_unpublish examples/publish_unpublish.cpp)
    target_link_libraries(example_publish_unpublish PRIVATE yandex-disk-cpp-client)

    add_executable(example_move_rename examples/move_rename.cpp)
    target_link_libraries(example_move_rename PRIVATE yandex-disk-cpp-client)

    add_executable(example_search_files examples/search_files.cpp)
    target_link_libraries(example_search_files PRIVATE yandex-disk-cpp-client)

    add_executable(example_directory_upload_download examples/directory_upload_download.cpp)
    target_link_libraries(example_directory_upload_download PRIVATE yandex-disk-cpp-client)
endif()

# === Installing a static library ===

install(TARGETS yandex-disk-cpp-client
        EXPORT yandex-disk-cpp-client-targets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
)

# === Copying all public headers ===

install(DIRECTORY include/ DESTINATION include)

# === Exporting targets for find_package() ===

install(EXPORT yandex-disk-cpp-client-targets
        FILE yandex-disk-cpp-clientTargets.cmake
        NAMESPACE yandex-disk-cpp-client::
        DESTINATION lib/cmake/yandex-disk-cpp-client
)

# === CMake package config & version files ===

include(CMakePackageConfigHelpers)
configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/yandex-disk-cpp-clientConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/yandex-disk-cpp-clientConfig.cmake"
        INSTALL_DESTINATION lib/cmake/yandex-disk-cpp-client
)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/yandex-disk-cpp-clientConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)
install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/yandex-disk-cpp-clientConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/yandex-disk-cpp-clientConfigVersion.cmake"
        DESTINATION lib/cmake/yandex-disk-cpp-client
)
