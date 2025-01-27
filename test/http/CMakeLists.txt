cmake_minimum_required(VERSION 3.18)

include(GoogleTest)

enable_testing()

add_executable(HTTP1_test http1.0.cpp)
target_link_libraries(HTTP1_test PUBLIC
    ProtocolHandler
    GTest::gtest_main
)

gtest_discover_tests(HTTP1_test)

# Optional: Install test executable
install(
    TARGETS HTTP1_test
    RUNTIME DESTINATION bin
)
