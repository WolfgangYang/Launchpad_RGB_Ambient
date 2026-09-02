cmake_minimum_required(VERSION 3.15)
project(LaunchpadRGBAmbient VERSION 0.2.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 源文件列表 (显式列出，确保大小写与文件名完全一致) ---
set(SOURCES
    src/main.cpp
    src/MIDI/MIDIDevice.cpp
    src/LED/LEDController.cpp
    src/Effects/Effect.cpp
    src/UI/Page.cpp
    src/Config/Config.cpp
    src/Renderer/Renderer.cpp
)

# --- 创建可执行文件 ---
add_executable(${PROJECT_NAME} ${SOURCES})

# --- 链接 Windows 系统库 ---
target_link_libraries(${PROJECT_NAME}
    winmm      # MIDI 支持
    comctl32   # UI 控件
    pdh        # 系统性能计数器 (CPU/RAM)
    d2d1       # Direct2D 渲染
    dwrite     # DirectWrite 文字
    ole32      # OLE 组件
)

# --- 头文件包含路径 ---
target_include_directories(${PROJECT_NAME} PRIVATE src)

# --- 编译器设置 (MSVC 环境下开启警告) ---
if(MSVC)
    add_compile_options(/W4 /WX-)
endif()
