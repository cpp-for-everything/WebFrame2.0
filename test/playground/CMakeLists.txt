cmake_minimum_required(VERSION 3.18)

add_executable(Playground main.cpp)
target_link_libraries(Playground PUBLIC BootServer ProtocolHandler)
