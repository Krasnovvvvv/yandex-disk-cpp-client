cmake_minimum_required(VERSION 3.28)
find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

add_executable(yandex-disk-cpp-client-tests
        test_path_utils.cpp
        test_format_utils.cpp
        test_directory_ops.cpp
        test_search_utils.cpp
)

target_link_libraries(yandex-disk-cpp-client-tests
        PRIVATE
        yandex-disk-cpp-client
        GTest::gtest_main
)

gtest_discover_tests(yandex-disk-cpp-client-tests)