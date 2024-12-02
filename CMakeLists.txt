cmake_minimum_required(VERSION 3.20)
project(WebFrame VERSION 1.0.0.0 LANGUAGES CXX)

option(TEMPLATE_ENGINE "Template engine to be used." OFF)
option(WITH_TESTS "Compile unit and integration tests targets" ON)
option(WITH_BENCHMARKS "Compile benchmark targets" ON)

set(CMAKE_CXX_STANDARD 23)
set(CMKAE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(sync_log)
add_subdirectory(protocol)