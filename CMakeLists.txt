cmake_minimum_required(VERSION 3.16)
project(blackjack)

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE AND NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false CONFIGURE_DEPENDS
    "src/*.cpp" "src/*.hpp")

add_executable(blackjack main.cpp ${SRC_FILES})
