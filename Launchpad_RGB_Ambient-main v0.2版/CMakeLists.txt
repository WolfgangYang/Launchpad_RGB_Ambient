cmake_minimum_required(VERSION 3.15)
project(LaunchpadRGBAmbient VERSION 0.2.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
aux_source_directory(src/main.cpp MAIN_SRCS)
aux_source_directory(src/MIDI MIDI_SRCS)
aux_source_directory(src/LED LED_SRCS)
aux_source_directory(src/Effects EFFECTS_SRCS)
aux_source_directory(src/UI UI_SRCS)
aux_source_directory(src/Config CONFIG_SRCS)
aux_source_directory(src/Renderer RENDERER_SRCS)

# Create executable
add_executable(${PROJECT_NAME}
    ${MAIN_SRCS}
    ${MIDI_SRCS}
    ${LED_SRCS}
    ${EFFECTS_SRCS}
    ${UI_SRCS}
    ${CONFIG_SRCS}
    ${RENDERER_SRCS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    winmm
    comctl32
    pdh
    d2d1
    dwrite
    ole32
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE src)

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /WX-)
endif()