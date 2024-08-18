cmake_minimum_required(VERSION 3.28)
project(lunar)

set(LUNAR_BUILD_EXAMPLE ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(json GIT_REPOSITORY https://github.com/nlohmann/json.git)
FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git)
FetchContent_MakeAvailable(json glm)

find_package(JNI REQUIRED)

add_library(lunar SHARED
    "inc/lunar/utils/argument_parser.hpp"
    "inc/lunar/file/filesystem.hpp"
    "inc/lunar/file/text_file.hpp"
    "inc/lunar/file/json_file.hpp"
    "inc/lunar/utils/lexer.hpp"
    "inc/lunar/debug/log.hpp"
    "inc/lunar/debug/assert.hpp"
    "inc/lunar/ui/stylesheet.hpp"
    "inc/lunar/core/scene.hpp"
    "inc/lunar/core/gameobject.hpp"
    "inc/lunar/core/component.hpp"
    "inc/lunar/script/script_vm.hpp"
    "inc/lunar/api.hpp"

    "src/utils/argument_parser.cpp"
    "src/file/filesystem.cpp"
    "src/file/text_file.cpp"
    "src/file/json_file.cpp"
    "src/utils/lexer.cpp"
    "src/debug/log.cpp"
    "src/ui/stylesheet.cpp"
    "src/core/scene.cpp"
    "src/core/gameobject.cpp"
    "src/core/component.cpp"
    "src/script/script_vm.cpp"
)
target_include_directories(lunar PUBLIC inc)
target_include_directories(lunar SYSTEM INTERFACE nlohmann_json JNI::JNI JNI::JVM glm)
target_link_libraries(lunar PUBLIC nlohmann_json JNI::JNI JNI::JVM glm)

target_compile_definitions(lunar PRIVATE
    DEBUG_JVM_VERBOSE=0
    LUNAR_LIBRARY_EXPORT
)

add_custom_target(lunar_bundler COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(lunar lunar_bundler)

if(${LUNAR_BUILD_EXAMPLE})
    add_executable(lunar-example "examples/app.cpp")
    target_include_directories(lunar-example INTERFACE lunar)
    target_link_libraries(lunar-example PUBLIC lunar)
    message("-- Building example app")
endif()

if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static")
    message("-- Found MinGW build environment.")
endif()
