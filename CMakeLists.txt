cmake_minimum_required(VERSION 3.15)
project(LaunchpadRGBAmbient VERSION 0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Windows Specific Settings ---
if(MSVC)
    add_compile_options(/W4 /MP)
endif()

# --- Third-Party Libraries (ImGui & DirectX) ---
include(FetchContent)

# ImGui - Fetching the core library
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1907
)
FetchContent_MakeAvailable(imgui)

# --- Project Include Directories ---
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# --- Source Files ---
file(GLOB_RECURSE SOURCES "src/*.cpp")

# --- Windows System Libraries ---
if(WIN32)
    set(PLATFORM_LIBS 
        winmm.lib 
        comctl32.lib 
        pdh.lib 
        d3d11.lib 
        dxgi.lib 
        dwmapi.lib 
        gdi32.lib 
        shell32.lib
    )
endif()

# --- Executable Target ---
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE ${PLATFORM_LIBS} imgui::imgui)

# Set output directory to build/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
