cmake_minimum_required(VERSION 3.28)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_COMPILER clang++-latest)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)

project(circle-lang)

find_package(boost_program_options REQUIRED CONFIG)

file(GLOB_RECURSE src "src/*.cpp")

add_executable(circle-lang ${src})

target_link_libraries(circle-lang PRIVATE Boost::program_options)
