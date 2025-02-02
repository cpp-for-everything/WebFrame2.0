cmake_minimum_required(VERSION 3.15)

include(FetchContent)

FetchContent_Declare(
  zlib
  GIT_REPOSITORY https://github.com/madler/zlib.git # Or wherever zlib is hosted
  GIT_TAG v1.3.1 # Specify a tag or commit
)

FetchContent_MakeAvailable(zlib)
