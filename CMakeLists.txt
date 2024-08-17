cmake_minimum_required(VERSION 3.28)
project(lunar)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(json GIT_REPOSITORY https://github.com/nlohmann/json.git)
FetchContent_MakeAvailable(json)

find_package(JNI REQUIRED)

add_executable(lunar
    "src/utils/argument_parser.cpp"
    "inc/utils/argument_parser.hpp"
    "inc/file/filesystem.hpp"
    "src/file/filesystem.cpp"
    "inc/file/text_file.hpp"
    "src/file/text_file.cpp"
    "inc/file/json_file.hpp"
    "src/file/json_file.cpp"
    "inc/utils/lexer.hpp"
    "src/utils/lexer.cpp"
    "inc/debug/log.hpp"
    "src/debug/log.cpp"
    "inc/debug/assert.hpp"
    "inc/ui/stylesheet.hpp"
    "src/ui/stylesheet.cpp"

    "inc/core/scene.hpp"
    "src/core/scene.cpp"
    "inc/core/gameobject.hpp"
    "src/core/gameobject.cpp"
    "inc/core/component.hpp"
    "src/core/component.cpp"

    "inc/script/script_vm.hpp"
    "src/script/script_vm.cpp"

    "src/main.cpp"
)
target_include_directories(lunar PUBLIC inc)
target_include_directories(lunar SYSTEM INTERFACE nlohmann_json JNI::JNI JNI::JVM)
target_link_libraries(lunar PRIVATE nlohmann_json JNI::JNI JNI::JVM)

target_compile_definitions(lunar PRIVATE
    DEBUG_JVM_VERBOSE=0
)

add_custom_target(lunar_bundler COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(lunar lunar_bundler)
