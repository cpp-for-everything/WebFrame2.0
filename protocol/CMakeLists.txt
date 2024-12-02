cmake_minimum_required(VERSION 3.20)

add_library(WebFrame_Protocol_Interface INTERFACE inc/interface.h inc/platform.h)
target_compile_features(WebFrame_Protocol_Interface INTERFACE cxx_std_23)
target_include_directories(WebFrame_Protocol_Interface INTERFACE inc/)

add_library(WebFrame_Protocol_HTTP_1_0 STATIC inc/http/1.0.hpp src/http/1.0.cpp)
target_compile_features(WebFrame_Protocol_HTTP_1_0 PUBLIC cxx_std_23)
target_link_libraries(WebFrame_Protocol_HTTP_1_0 PUBLIC WebFrame_Protocol_Interface)