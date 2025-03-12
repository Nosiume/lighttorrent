cmake_minimum_required(VERSION 3.15)

project(lighttorrent VERSION 1.0 LANGUAGES CXX DESCRIPTION "A lightweight bittorrent client in c++")


set( CMAKE_CXX_STANDARD 20 )
set( PROJECT_SOURCE_DIR src/ )
set( PROJECT_INCLUDE_DIR include/ )

file( GLOB_RECURSE sources ${PROJECT_SOURCE_DIR}/*.cpp )

add_executable( lighttorrent ${sources} )

target_include_directories( lighttorrent PUBLIC ${PROJECT_INCLUDE_DIR} )

# libs
find_package(Curses REQUIRED)
target_include_directories( lighttorrent PUBLIC ${CURSES_INCLUDE_DIR} )
target_link_libraries( lighttorrent ${CURSES_LIBRARY} )




