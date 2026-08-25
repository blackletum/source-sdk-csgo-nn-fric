CMAKE_MINIMUM_REQUIRED(VERSION 4.0)

project(Counter-Strike-Global-Offensive LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../game/bin)
set(SDL_STATIC ON)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/SDL/include)

add_subdirectory(SDL)
add_subdirectory(interface)
add_subdirectory(vaudio)
