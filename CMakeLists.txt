cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(circle-lang)

enable_testing()

find_package(boost_program_options CONFIG REQUIRED)

file(GLOB_RECURSE src "src/*.cpp")

add_executable(circle-lang ${src})

target_link_libraries(circle-lang PRIVATE Boost::program_options)

# find_package(GTest CONFIG REQUIRED)
#
# file(GLOB_RECURSE src "")
#
# target_link_libraries(circle-lang PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
#
# add_test(AllTestsInMain circle-lang)
