cmake_minimum_required(VERSION 3.18)

add_executable(HTTP1_test http1.0.cpp)
target_link_libraries(HTTP1_test
    PUBLIC
    ProtocolHandler
    PRIVATE
    Catch2::Catch2WithMain
    gmock
    gmock_main
)

# Enable Catch2 test discovery
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/Catch2/extras")
include(Catch)
catch_discover_tests(HTTP1_test)
