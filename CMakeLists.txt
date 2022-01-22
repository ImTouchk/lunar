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

add_executable("lunar" "src/main.cpp")
target_include_directories("lunar" PUBLIC inc)
target_compile_definitions("lunar" PRIVATE
        PRINT_DEBUG_OUTPUT=1
        WINDOW_BACKEND=${WINDOW_BACKEND}
        RENDERER_BACKEND=${RENDERER_BACKEND}
)

if(COUNT_HEADERS_AS_SOURCE)
    target_sources("lunar" PRIVATE
        "inc/render/window.hpp"
        "inc/render/renderer.hpp"
    )
endif()

if(WINDOW_BACKEND STREQUAL "GLFW")
    message("-- Using GLFW as the windowing backend.")

    add_subdirectory(glfw)
    target_link_libraries("lunar" glfw)
    target_include_directories("lunar" PRIVATE glfw)

    target_sources("lunar" PRIVATE "src/glfw_window.cpp")
endif()

if(RENDERER_BACKEND STREQUAL "Vulkan")
    message("-- Using Vulkan as the rendering backend.")

    find_package(Vulkan REQUIRED FATAL_ERROR)
    target_include_directories("lunar" PRIVATE ${Vulkan_INCLUDE_DIRS})
    target_link_libraries("lunar" ${Vulkan_LIBRARIES})

    target_compile_definitions("lunar" PRIVATE VULKAN_USE_DEBUG_LAYERS=1)

    target_sources("lunar" PRIVATE
        "src/vk_renderer.hpp"
        "src/vk_renderer.cpp"
        "src/vk_instance.cpp"
    )
endif()

add_subdirectory(fmt)
target_link_libraries("lunar" fmt)
target_include_directories("lunar" PRIVATE fmt)
