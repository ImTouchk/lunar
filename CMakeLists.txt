cmake_minimum_required(VERSION 3.29 FATAL_ERROR)
project(lunar VERSION 0.1.9)

message("-- lunar: Version ${lunar_VERSION_MAJOR}.${lunar_VERSION_MINOR}.${lunar_VERSION_PATCH}")

set(LUNAR_BUILD_EXAMPLE ON)
set(LUNAR_BUNDLE_RESOURCES ON)
set(LUNAR_RENDER_BACKEND "VULKAN")
set(LUNAR_RESOURCES_DIR ${CMAKE_CURRENT_LIST_DIR}/resources)
set(LUNAR_ASSETS_DIR ${LUNAR_RESOURCES_DIR}/data)
set(LUNAR_RUNTIME_SHADER_BUILD OFF)
set(LUNAR_EXPERIMENTAL ON)

set(CMAKE_CXX_STANDARD 23)
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
    "inc/lunar/file/file_tracker.hpp"

    "inc/lunar/utils/lexer.hpp"
    "inc/lunar/utils/stopwatch.hpp"
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

    "src/utils/stopwatch.cpp"
    "src/utils/argument_parser.cpp"
    "src/utils/identifiable.cpp"
    "src/file/filesystem.cpp"
    "src/file/text_file.cpp"
    "src/file/json_file.cpp"
    "src/file/config_file.cpp"
    "src/file/binary_file.cpp"
    "src/file/file_tracker.cpp"
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

    "inc/lunar/render/mesh.hpp"
    "inc/lunar/render/image.hpp"
    "inc/lunar/render/window.hpp"
    "inc/lunar/render/shader.hpp"
    "inc/lunar/render/common.hpp"
    "inc/lunar/render/render_context.hpp"
    "inc/lunar/render/render_components.hpp"
    "src/render/render_components.cpp"
    
    "inc/lunar/render/terra.hpp"
    "inc/lunar/render/terra/expression.hpp"
    "inc/lunar/render/terra/statement.hpp"
    "inc/lunar/render/terra/parser.hpp"
    "inc/lunar/render/terra/transpiler.hpp"
    
    "src/render/terra/parser.cpp"
    "src/render/terra/transpiler.cpp"
    "src/render/terra/type_checker.cpp"
    "src/render/terra/vk_transpiler.cpp"

    "src/render/image.cpp"
    "src/render/window.cpp"
    "src/render/shader.cpp"
)

if(${LUNAR_EXPERIMENTAL})
    message("-- lunar: Including experimental features")
    target_sources(lunar PRIVATE
        "inc/lunar/exp/utils/token.hpp"
        "inc/lunar/exp/utils/lexer.hpp"
        "src/exp/utils/lexer.cpp"

        "inc/lunar/exp/utils/scanner.hpp"
        "src/exp/utils/scanner.cpp"
        
        "inc/lunar/exp/ui/dom.hpp"
        "src/exp/ui/dom.cpp"
    )
    target_compile_definitions(lunar PUBLIC LUNAR_EXPERIMENTAL)
endif()

if(${LUNAR_RENDER_BACKEND} STREQUAL "VULKAN")
    FetchContent_Declare(vma GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git)
    FetchContent_MakeAvailable(vma)
    find_package(Vulkan REQUIRED)

    message("-- lunar: Using Vulkan backend")
    target_sources(lunar PRIVATE
        "src/render/vk_draw.cpp"
        "src/render/vk_base.cpp"
        "src/render/vk_image.cpp"
        "src/render/vk_shader.cpp"
        "src/render/vk_window.cpp"
        "src/render/vk_buffer.cpp"
        "inc/lunar/render/internal/render_vk.hpp"
    )
    target_compile_definitions(lunar PRIVATE
        VULKAN_HPP_NO_CONSTRUCTORS  
        LUNAR_VERBOSE
        LUNAR_RUNTIME_SHADER_BUILD=${LUNAR_RUNTIME_SHADER_BUILD}
    )
    target_compile_definitions(lunar PUBLIC LUNAR_VULKAN)
    target_include_directories(lunar SYSTEM INTERFACE Vulkan::Vulkan VulkanMemoryAllocator)
    target_link_libraries(lunar PUBLIC Vulkan::Vulkan VulkanMemoryAllocator)

    if(NOT ${LUNAR_RUNTIME_SHADER_BUILD})
        message("-- lunar: Compiling Vulkan shaders at build time")

        file(GLOB SHADER_FILES 
            ${LUNAR_ASSETS_DIR}/shader-src/*.vert
            ${LUNAR_ASSETS_DIR}/shader-src/*.frag
            ${LUNAR_ASSETS_DIR}/shader-src/*.comp
        )

        foreach(SHADER_FILE IN LISTS SHADER_FILES)
            get_filename_component(SHADER_FILE_NAME ${SHADER_FILE} NAME)
            add_custom_command(
                OUTPUT ${LUNAR_ASSETS_DIR}/shader-bin/${SHADER_FILE_NAME}.spv
                COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER_FILE} -o ${LUNAR_ASSETS_DIR}/shader-bin/${SHADER_FILE_NAME}.spv
                DEPENDS ${SHADER_FILE}
                COMMENT "-- lunar: Compiling Vulkan shader '${SHADER_FILE_NAME}'"
            )
            list(APPEND LUNAR_COMPILED_SHADERS ${LUNAR_ASSETS_DIR}/shader-bin/${SHADER_FILE_NAME}.spv)
        endForeach()

        add_custom_target(lunar_shaders ALL DEPENDS ${LUNAR_COMPILED_SHADERS})
        add_dependencies(lunar lunar_shaders)

        unset(SHADER_FILES)
    else()
        set(LUNAR_COMPILED_SHADERS "")
    endif()
endif()

target_include_directories(lunar PUBLIC inc)
target_include_directories(lunar SYSTEM INTERFACE nlohmann_json JNI::JNI JNI::JVM glm glfw)
target_link_libraries(lunar PUBLIC nlohmann_json JNI::JNI JNI::JVM glm glfw)

target_compile_definitions(lunar PRIVATE 
    LUNAR_LIBRARY_EXPORT
)

target_compile_definitions(lunar PUBLIC 
    LUNAR_RENDER_BACKEND="${LUNAR_RENDER_BACKEND}"
    LUNAR_VER_MAJOR=${lunar_VERSION_MAJOR}
    LUNAR_VER_MINOR=${lunar_VERSION_MINOR}
    LUNAR_VER_PATCH=${lunar_VERSION_PATCH}
)

if(MSVC)
    target_compile_options(lunar PUBLIC /Zc:preprocessor)
    message("-- lunar: Detected MSVC compiler")
endif()

if(${LUNAR_BUNDLE_RESOURCES})
    add_custom_target(lunar_bundler 
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}
    )
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
