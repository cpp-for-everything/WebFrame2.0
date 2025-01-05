cmake_minimum_required(VERSION 3.18)

add_library(ProtocolHandler STATIC protocol_handler.cpp)
target_sources(ProtocolHandler
    PUBLIC FILE_SET HEADERS TYPE HEADERS FILES protocol_handler.h http/common/status_code.h
)
target_include_directories(ProtocolHandler PUBLIC ${CMAKE_SOURCE_DIR}/src)