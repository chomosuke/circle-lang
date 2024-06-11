cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_COMPILER clang-18)
set(CMAKE_C_COMPILER clang-18)
set(CMAKE_MAKE_PROGRAM ninja)

project(CircleLang)

find_package(boost CONFIG REQUIRED)

add_executable(CircleLang main.cpp)

target_link_libraries(CircleLang PRIVATE boost::boost)
