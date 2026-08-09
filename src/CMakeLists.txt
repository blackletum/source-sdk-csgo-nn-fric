CMAKE_MINIMUM_REQUIRED(VERSION 4.0)

project(Counter-Strike-Global-Offensive LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../game/bin)

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(interface)
