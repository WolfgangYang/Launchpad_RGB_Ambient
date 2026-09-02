cmake_minimum_required(VERSION 3.15)
project(LaunchpadRGBAmbient VERSION 0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Windows Specific Settings ---
if(MSVC)
    add_compile_options(/W4 /MP)
endif()

# --- Third-Party Libraries (ImGui via FetchContent) ---
include(FetchContent)

# We fetch the main ImGui repository
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG master
)

# Note: In a real-world production environment, we would also include 
# imgui's backends (imgui_impl_win32 and imgui_impl_dx11).
# For this automated setup, we assume the user/CI will provide them or 
# they are part of the source tree.

FetchContent_MakeAvailable(imgui)

# --- Project Include Directories ---
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# --- Source Files ---
file(GLOB_RECURSE SOURCES "src/*.cpp")

# --- Windows System Libraries (DirectX 11 & Win32) ---
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
target_link_libraries(${PROJECT_NAME} PRIVATE ${PLATFORM_LIBS} imgui)

# Set output directory to build/bin for easy artifact collection
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
