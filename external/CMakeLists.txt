cmake_minimum_required(VERSION 3.15)

include(FetchContent)

add_subdirectory(Catch2)
add_subdirectory(GoogleTest)
add_subdirectory(RegexMatcher)
add_subdirectory(nghttp2)
add_subdirectory(zlib)

FetchContent_MakeAvailable(Catch2 GoogleTest RegexMatcher nghttp2 zlib)
