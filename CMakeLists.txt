cmake_minimum_required(VERSION 3.21)

project("lunar")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(COUNT_HEADERS_AS_SOURCE 1)
set(WINDOW_BACKEND "GLFW")

find_package(Vulkan REQUIRED FATAL_ERROR)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)
add_subdirectory(fmt)

list(APPEND SOURCE_FILES
    "src/main.cpp"
)

if(COUNT_HEADERS_AS_SOURCE)
    list(APPEND SOURCE_FILES
        "inc/window.hpp"
    )
endif()

if(WINDOW_BACKEND STREQUAL "GLFW")
    list(APPEND SOURCE_FILES
        "src/glfw_window.cpp"
    )
endif()

add_executable("lunar" ${SOURCE_FILES})
target_link_libraries("lunar" ${Vulkan_LIBRARIES} glfw fmt)
target_include_directories("lunar" PRIVATE ${Vulkan_INCLUDE_DIRS} glfw fmt)
target_include_directories("lunar" PUBLIC inc)
target_compile_definitions("lunar" PRIVATE PRINT_DEBUG_OUTPUT)