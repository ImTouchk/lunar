cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(lunixc)
target_sources(lunixc PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
)