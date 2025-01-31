cmake_minimum_required(VERSION 3.15)

include(FetchContent)

FetchContent_Declare(
    Chromium
    GIT_REPOSITORY https://chromium.googlesource.com/chromium.git
    GIT_TAG ca4965ce7e9ed7ce92685976c0d622151be52fb8 # refs/tags/134.0.6988.1
)
