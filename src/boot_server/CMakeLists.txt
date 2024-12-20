cmake_minimum_required(VERSION 3.18)

add_library(BootServerInterface INTERFACE)
#target_sources(BootServerInterface
#    INTERFACE FILE_SET HEADERS TYPE HEADERS FILES iserver.h
#)

add_library(BootServer STATIC server.cpp)
#target_sources(BootServer
#    PUBLIC FILE_SET HEADERS TYPE HEADERS FILES server.h protocol_manager.h platform.h
#)
target_link_libraries(BootServer PUBLIC BootServerInterface)
target_include_directories(BootServer PUBLIC ${CMAKE_SOURCE_DIR}/src)