cmake_minimum_required(VERSION 3.15)

if(TARGET nghttp2_static)
    message("Target available")
endif()

message(STATUS "${nghttp2_SOURCE_DIR}/lib/includes ${nghttp2_BINARY_DIR}/lib/includes")

# Your parser library
add_executable(nghttp2_http2_parser parser.cpp)

# Link using the correct target properties
target_include_directories(nghttp2_http2_parser PRIVATE
    ${nghttp2_SOURCE_DIR}/lib/includes
    ${nghttp2_BINARY_DIR}/lib/includes
)

target_link_libraries(nghttp2_http2_parser PRIVATE
    nghttp2_static # Use static library target name
)

# For MSVC specifically
if(MSVC)
    target_compile_options(nghttp2_http2_parser PRIVATE
        /wd4267 # Disable size_t to int conversion warnings
        /wd4018 # Disable signed/unsigned mismatch
    )
endif()
