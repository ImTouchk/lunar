cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LUNAR_COUNT_HEADERS_AS_SOURCE 1)
set(LUNAR_WINDOW_BACKEND "GLFW")
set(LUNAR_RENDERER_BACKEND "Vulkan")

set(LUNAR_WINDOW_BACKEND ${LUNAR_WINDOW_BACKEND} PARENT_SCOPE)
set(LUNAR_RENDERER_BACKEND ${LUNAR_RENDERER_BACKEND} PARENT_SCOPE)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_executable("lunar" 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/filesystem.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/thread_pool.cpp"
)
target_include_directories("lunar" PUBLIC inc)
target_compile_definitions("lunar" PRIVATE
    PRINT_DEBUG_OUTPUT=1
    WINDOW_BACKEND=${LUNAR_WINDOW_BACKEND}
    RENDERER_BACKEND=${LUNAR_RENDERER_BACKEND}
)

if(LUNAR_COUNT_HEADERS_AS_SOURCE)
    target_sources("lunar" PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/inc/render/window.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/inc/render/renderer.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/inc/render/mesh.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/inc/utils/debug.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/inc/utils/range.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/inc/utils/thread_pool.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/inc/utils/identifier.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/inc/io/filesystem.hpp"
        #"inc/math/vec.hpp"
    )
endif()

if(LUNAR_WINDOW_BACKEND STREQUAL "GLFW")
    message("-- Using GLFW as the windowing backend.")

    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/glfw")
    target_link_libraries("lunar" glfw)
    target_include_directories("lunar" PRIVATE glfw)

    target_sources("lunar" PRIVATE "src/glfw_window.cpp")
endif()

if(LUNAR_RENDERER_BACKEND STREQUAL "Vulkan")
    message("-- Using Vulkan as the rendering backend.")
    find_package(Vulkan REQUIRED FATAL_ERROR)
    find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc REQUIRED)
    set(glslc ${glslc_executable} PARENT_SCOPE)

    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator")
    target_include_directories("lunar" PRIVATE ${Vulkan_INCLUDE_DIRS} VulkanMemoryAllocator)
    target_link_libraries("lunar" ${Vulkan_LIBRARIES} VulkanMemoryAllocator)

    target_compile_definitions("lunar" PRIVATE VULKAN_USE_DEBUG_LAYERS=1)

    target_sources("lunar" PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vk_gpu.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vk_sync.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vk_device.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vk_shader.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vk_surface.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vk_renderer.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vk_renderer.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vk_instance.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vk_swapchain.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vk_shader_manager.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vk_buffer.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vk_buffer.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vk_object.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vk_object.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vk_draw_call.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vk_draw_call.cpp"
        #"${CMAKE_CURRENT_SOURCE_DIR}/src/vk_mesh.cpp"
    )
endif()

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS  "-static")
endif()

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/fmt")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/glm")
target_link_libraries("lunar" fmt glm)
target_include_directories("lunar" PRIVATE fmt glm)

set(LUNAR_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" PARENT_SCOPE)
