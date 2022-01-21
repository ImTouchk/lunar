cmake_minimum_required(VERSION 3.21)

project("lunar")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COUNT_HEADERS_AS_SOURCE 1)
set(WINDOW_BACKEND "GLFW")
set(RENDERER_BACKEND "Vulkan")

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
        "inc/render/window.hpp"
        "inc/render/renderer.hpp"
    )
endif()

if(WINDOW_BACKEND STREQUAL "GLFW")
    message("-- Using GLFW as the windowing backend.")
    list(APPEND SOURCE_FILES "src/glfw_window.cpp")
endif()

if(RENDERER_BACKEND STREQUAL "Vulkan")
    message("-- Using Vulkan as the rendering backend.")
    list(APPEND SOURCE_FILES
        "src/vk_renderer.cpp"
        "src/vk_renderer.hpp"
        "src/vk_instance.cpp"
    )
endif()

add_executable("lunar" ${SOURCE_FILES})
target_include_directories("lunar" PUBLIC inc)
target_compile_definitions("lunar" PRIVATE PRINT_DEBUG_OUTPUT)

target_link_libraries("lunar" glfw fmt)
target_include_directories("lunar" PRIVATE glfw fmt)

target_compile_definitions("lunar" PRIVATE WINDOW_BACKEND=${WINDOW_BACKEND})
target_compile_definitions("lunar" PRIVATE RENDERER_BACKEND=${RENDERER_BACKEND})

if(RENDERER_BACKEND STREQUAL "Vulkan")
    find_package(Vulkan REQUIRED FATAL_ERROR)
    target_include_directories("lunar" PRIVATE ${Vulkan_INCLUDE_DIRS})
    target_link_libraries("lunar" ${Vulkan_LIBRARIES})
endif()