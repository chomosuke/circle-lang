cmake_minimum_required(VERSION 3.28)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_COMPILER /home/rli43/code/llvm-project/build/bin/clang++)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
# set(CMAKE_CXX_FLAGS "-fexperimental-modules-reduced-bmi")

project(circle-lang)

find_package(boost_program_options REQUIRED CONFIG)

file(GLOB_RECURSE src src/*.cppm src/main.cpp)
add_executable(circle-lang ${src})

file(GLOB_RECURSE modules src/*.cppm)
target_sources(circle-lang PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES ${modules})

target_link_libraries(circle-lang PRIVATE Boost::program_options)
