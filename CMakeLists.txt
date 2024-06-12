cmake_minimum_required(VERSION 3.22)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_COMPILER clang++-18)
set(CMAKE_C_COMPILER clang-18)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)

project(circle-lang)

find_package(boost_program_options REQUIRED CONFIG)

add_executable(circle-lang src/main.cpp)

target_link_libraries(circle-lang PRIVATE Boost::program_options)
