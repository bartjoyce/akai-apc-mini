cmake_minimum_required(VERSION 2.8)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.10" CACHE STRING "Minimum OS X deployment version")
project(akai-apc-mini)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(lib/rtmidi)

add_executable(akai-apc-mini
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/rtmidi/
)

target_link_libraries(akai-apc-mini rtmidi)
