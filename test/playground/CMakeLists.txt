cmake_minimum_required(VERSION 3.20)

add_executable(Playground main.cpp)
target_link_libraries(Playground PUBLIC ServerBoot ProtocolHandler)