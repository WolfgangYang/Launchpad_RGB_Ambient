cmake_minimum_required(VERSION 3.10)
project(LaunchpadRGBAmbient VERSION 0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置 Windows 特有的字符集 (Unicode)
add_definitions(-DUNICODE -D_UNICODE)

# 获取源文件列表
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 设置包含目录，确保能找到 src 下的所有模块头文件
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

# 针对 Windows 环境的稳健链接方式
if(WIN32)
    # 直接使用系统库名称。在 GitHub Actions 的 MSVC 环境下，这是最可靠的方式。
    target_link_libraries(${PROJECT_NAME} PRIVATE winmm comctl32 pdh)
endif()

# 强制设置输出目录为 bin/Release，确保与 workflow 中的 path 完全一致
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/Release")

message(STATUS "Project configured for Windows. Libraries: winmm, comctl32, pdh")
