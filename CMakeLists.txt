cmake_minimum_required(VERSION 3.20)
project(Launchpad_RGB_Ambient VERSION 0.4.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(Launchpad_RGB_Ambient WIN32
    src/main.cpp
    src/app/application.cpp
    src/core/color.cpp
    src/effects/effect_engine.cpp
    src/hardware/midi_output.cpp
    src/localization/localization.cpp
    src/monitor/system_monitor.cpp
    src/ui/main_window.cpp
)

target_compile_definitions(Launchpad_RGB_Ambient PRIVATE UNICODE _UNICODE NOMINMAX)

target_compile_options(Launchpad_RGB_Ambient PRIVATE /utf-8 /W4 /EHsc)

target_link_libraries(Launchpad_RGB_Ambient PRIVATE
    user32
    gdi32
    winmm
    comctl32
    pdh
)
