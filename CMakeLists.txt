cmake_minimum_required(VERSION 3.21)

project("lunar")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED FATAL_ERROR)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

list(APPEND SOURCE_FILES
    "src/main.cpp"
)

add_executable("lunar" ${SOURCE_FILES})
target_link_libraries("lunar" ${Vulkan_LIBRARIES} glfw)
target_include_directories("lunar" PRIVATE ${Vulkan_INCLUDE_DIRS} glfw)
target_include_directories("lunar" PUBLIC inc)
