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

add_test(NAME HTTP1_test COMMAND HTTP1_test)
