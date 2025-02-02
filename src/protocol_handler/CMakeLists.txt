cmake_minimum_required(VERSION 3.18)

add_library(HTTP1.0 STATIC http/1.0.cpp http/1.0.h)
target_include_directories(HTTP1.0 PUBLIC ${CMAKE_SOURCE_DIR}/src)

add_library(ProtocolHandler STATIC protocol_handler.cpp protocol_handler.h http/common/status_code.h)
target_include_directories(ProtocolHandler PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(ProtocolHandler PUBLIC HTTP1.0)
