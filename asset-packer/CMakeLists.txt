cmake_minimum_required(VERSION 3.21)
project(asset-packer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(asset-packer)
target_sources(asset-packer PUBLIC "src/main.cpp" "src/pack.cpp" "src/unpack.cpp")

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "-static")
endif()
