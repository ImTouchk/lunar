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

file(DOWNLOAD "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h" "inc/utils/stb_image.h")

add_executable("lunar" 
    "src/main.cpp" 
    "src/filesystem.cpp" 
    "src/thread_pool.cpp"
)

if(WIN32)
    target_sources("lunar" PRIVATE "src/w32_fast_mutex.cpp")
endif()

target_include_directories("lunar" PUBLIC inc)
target_compile_definitions("lunar" PRIVATE
    PRINT_DEBUG_OUTPUT=1
    WINDOW_BACKEND=${LUNAR_WINDOW_BACKEND}
    RENDERER_BACKEND=${LUNAR_RENDERER_BACKEND}
)

if(LUNAR_COUNT_HEADERS_AS_SOURCE)
    target_sources("lunar" PRIVATE
        "inc/render/window.hpp"
        "inc/render/mesh.hpp"
        "inc/render/shader.hpp"
        "inc/render/renderer.hpp"
        "inc/render/texture.hpp"
        "inc/utils/debug.hpp"
        "inc/utils/range.hpp"
        "inc/utils/identifier.hpp"
        "inc/utils/thread_pool.hpp"
        "inc/utils/thread_safe_queue.hpp"
        "inc/io/filesystem.hpp"
        "inc/utils/fast_mutex.hpp"
        "inc/utils/stb_image.h"
        "inc/core/time.hpp"
        "inc/core/input.hpp"
    )
endif()

if(LUNAR_WINDOW_BACKEND STREQUAL "GLFW")
    message("-- Using GLFW as the windowing backend.")

    add_subdirectory("glfw")
    target_link_libraries("lunar" glfw)
    target_include_directories("lunar" PRIVATE glfw)

    target_compile_definitions("lunar" PRIVATE GLFW_WINDOW_BACKEND=1)
    target_sources("lunar" PRIVATE "src/glfw_window.cpp" "src/glfw_time.cpp" "src/glfw_input.cpp" "inc/core/glfw_keys.hpp")
endif()

if(LUNAR_RENDERER_BACKEND STREQUAL "Vulkan")
    message("-- Using Vulkan as the rendering backend.")
    find_package(Vulkan REQUIRED FATAL_ERROR)
    find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc REQUIRED)
    set(glslc ${glslc_executable} PARENT_SCOPE)

    add_subdirectory("VulkanMemoryAllocator")
    target_include_directories("lunar" PRIVATE ${Vulkan_INCLUDE_DIRS} VulkanMemoryAllocator)
    target_link_libraries("lunar" ${Vulkan_LIBRARIES} VulkanMemoryAllocator)

    target_compile_definitions("lunar" PRIVATE VULKAN_USE_DEBUG_LAYERS=1 VULKAN_RENDERER=1 GLFW_INCLUDE_NONE=1)

    target_sources("lunar" PRIVATE
        "src/graphics/vk_device.cpp"
        "src/graphics/vk_surface.cpp"
        "src/graphics/vk_renderer.hpp"
        "src/graphics/vk_renderer.cpp"
        "src/graphics/vk_instance.cpp"
        "src/graphics/vk_swapchain.cpp"
        "src/graphics/vk_shader.hpp"
        "src/graphics/vk_shader.cpp"
        "src/graphics/vk_buffer.cpp"
        "src/graphics/vk_buffer.hpp"
        "src/graphics/vk_object.hpp"
        "src/graphics/vk_object.cpp"
        "src/graphics/vk_cmd_submitter.cpp"
        "src/graphics/vk_texture.hpp"
        "src/graphics/vk_texture.cpp"
        "src/graphics/vk_utils.hpp"
    )
elseif(LUNAR_RENDERER_BACKEND STREQUAL "OpenGL")
    message("-- Using OpenGL as the rendering backend")

    add_subdirectory("glad")
    target_link_libraries("lunar" glad)
    target_include_directories("lunar" PRIVATE glad)
    target_compile_definitions("lunar" PRIVATE OPENGL_RENDERER=1)

    target_sources("lunar" PRIVATE
        "src/graphics/gl_renderer.hpp"
        "src/graphics/gl_renderer.cpp"
    )    
endif()

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS  "-static")
endif()

add_subdirectory("fmt")
add_subdirectory("glm")
target_link_libraries("lunar" fmt glm)
target_include_directories("lunar" PRIVATE fmt glm)

set(LUNAR_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" PARENT_SCOPE)
