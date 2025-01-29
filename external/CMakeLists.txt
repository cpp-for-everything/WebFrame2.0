cmake_minimum_required(VERSION 3.15)

# Add RegexMatcher, which includes its own gtest
add_subdirectory(RegexMatcher)

# Alias its gtest/gmock targets to avoid conflicts
set_target_properties(gtest PROPERTIES ALIAS RegexMatcher_gtest)
set_target_properties(gmock PROPERTIES ALIAS RegexMatcher_gmock)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static library for nghttp2")
set(BUILD_STATIC_LIBS ON CACHE BOOL "Do not build shared library for nghttp2")
add_subdirectory(nghttp2)

# Alias its gtest/gmock targets to avoid conflicts
set_target_properties(gtest PROPERTIES ALIAS nghttp2_gtest)
set_target_properties(gmock PROPERTIES ALIAS nghttp2_gmock)

add_subdirectory(Catch2)
add_subdirectory(googletest)