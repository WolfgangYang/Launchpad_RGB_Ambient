cmake_minimum_required(VERSION 3.10)
project(LaunchpadRGBAmbient VERSION 0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DUNICODE -D_UNICODE)

find_library(WINMM_LIB winmm)
find_library(COMCTL32_LIB comctl32)
find_library(PDH_LIB pdh)

# 使用引号包裹路径，防止文件夹名称中的空格导致构建失败
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${WINMM_LIB} ${COMCTL32_LIB} ${PDH_LIB})
endif()

# 确保输出目录结构清晰，并处理空格路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/Release")

message(STATUS "Project configured for Windows with libraries: ${WINMM_LIB}, ${COMCTL32_LIB}, ${PDH_LIB}")
