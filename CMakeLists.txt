cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set(VCPKG_OVERLAY_TRIPLETS ./triplets.cmake)
# set(CLANG_DEFAULT_CXX_STDLIB stdc++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
# set(VCPKG_CXX_FLAGS "${VCPKG_CXX_FLAGS} -stdlib=libc++")
# set(VCPKG_LINKER_FLAGS "${VCPKG_LINKER_FLAGS} -stdlib=libc++")

project(circle-lang LANGUAGES CXX)


file(GLOB_RECURSE lib CONFIGURE_DEPENDS "lib/*.cpp")
add_library(lib STATIC ${lib})


file(GLOB_RECURSE src CONFIGURE_DEPENDS "src/*.cpp")
add_executable(circle-lang ${src})
target_link_libraries(circle-lang PRIVATE lib)

find_package(boost_program_options CONFIG REQUIRED)
target_link_libraries(circle-lang PRIVATE Boost::program_options)


enable_testing()

file(GLOB_RECURSE all_tests CONFIGURE_DEPENDS "test/*.cpp")
add_executable(all_tests ${all_tests})
target_link_libraries(all_tests PRIVATE lib)

find_package(GTest CONFIG REQUIRED)
target_link_libraries(all_tests PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(all_tests)
