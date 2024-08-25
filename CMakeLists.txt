cmake_minimum_required(VERSION 3.28)
project(lunar)

set(LUNAR_BUILD_EXAMPLE ON)
set(LUNAR_BUNDLE_RESOURCES ON)
set(LUNAR_RENDER_BACKEND "VULKAN")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(json GIT_REPOSITORY https://github.com/nlohmann/json.git)
FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git)
FetchContent_MakeAvailable(json glm glfw)

find_package(JNI REQUIRED)

add_library(lunar SHARED
    "inc/lunar/utils/argument_parser.hpp"
    "inc/lunar/file/filesystem.hpp"
    "inc/lunar/file/text_file.hpp"
    "inc/lunar/file/json_file.hpp"
    "inc/lunar/file/config_file.hpp"

    "inc/lunar/utils/lexer.hpp"
    "inc/lunar/utils/identifiable.hpp"
    "inc/lunar/debug/log.hpp"
    "inc/lunar/debug/assert.hpp"
    "inc/lunar/debug.hpp"
    
    "inc/lunar/ui/stylesheet.hpp"

    "inc/lunar/core/scene.hpp"
    "inc/lunar/core/gameobject.hpp"
    "inc/lunar/core/component.hpp"

    "inc/lunar/script/script_vm.hpp"
    "inc/lunar/script/script_api.hpp"
    "inc/lunar/script/api/scene.hpp"
    "inc/lunar/script/api/math.hpp"

    "inc/lunar/api.hpp"

    "src/utils/argument_parser.cpp"
    "src/utils/identifiable.cpp"
    "src/file/filesystem.cpp"
    "src/file/text_file.cpp"
    "src/file/json_file.cpp"
    "src/file/config_file.cpp"
    "src/utils/lexer.cpp"
    "src/debug/log.cpp"
    "src/ui/stylesheet.cpp"
    "src/core/scene.cpp"
    "src/core/gameobject.cpp"
    "src/core/component.cpp"

    "src/script/script_vm.cpp"
    "src/script/script_scene.cpp"
    "src/script/script_gameobject.cpp"
    "src/script/script_transform.cpp"
    "src/script/vm_utils.cpp"

    "inc/lunar/render/window.hpp"
    "inc/lunar/render/render_context.hpp"
    "src/render/render_context.cpp"
    "src/render/window.cpp"

)

if(${LUNAR_RENDER_BACKEND} STREQUAL "VULKAN")
    find_package(Vulkan REQUIRED)

    message("-- lunar: Using Vulkan backend")
    target_sources(lunar PRIVATE
        "src/render/vk_base.cpp"
        "inc/lunar/render/internal/render_vk.hpp"
    )
    target_compile_definitions(lunar PRIVATE
        VULKAN_HPP_NO_CONSTRUCTORS
        LUNAR_VULKAN
        LUNAR_VERBOSE
    )
    target_include_directories(lunar SYSTEM INTERFACE Vulkan::Vulkan)
    target_link_libraries(lunar PUBLIC Vulkan::Vulkan)
endif()

target_include_directories(lunar PUBLIC inc)
target_include_directories(lunar SYSTEM INTERFACE nlohmann_json JNI::JNI JNI::JVM glm glfw)
target_link_libraries(lunar PUBLIC nlohmann_json JNI::JNI JNI::JVM glm glfw)

target_compile_definitions(lunar PRIVATE 
    LUNAR_LIBRARY_EXPORT
)

target_compile_definitions(lunar PUBLIC LUNAR_RENDER_BACKEND="${LUNAR_RENDER_BACKEND}")

if(${LUNAR_BUNDLE_RESOURCES})
    add_custom_target(lunar_bundler COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR})
    add_dependencies(lunar lunar_bundler)
    message("-- lunar: Copying files from 'resources' folder to build directory.")
endif()

if(${LUNAR_BUILD_EXAMPLE})
    add_executable(lunar-example "examples/app.cpp")
    target_include_directories(lunar-example INTERFACE lunar)
    target_link_libraries(lunar-example PUBLIC lunar)
    message("-- lunar: Building example app.")
endif()

if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static")
    message("-- lunar: Found MinGW build environment.")
endif()
