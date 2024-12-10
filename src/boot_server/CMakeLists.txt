cmake_minimum_required(VERSION 3.20)

add_library(ServerBoot STATIC server.cpp)
target_sources(ServerBoot
    PUBLIC FILE_SET HEADERS TYPE HEADERS FILES server.h protocol_manager.h platform.h
)
target_include_directories(ServerBoot PUBLIC ${CMAKE_SOURCE_DIR}/src)