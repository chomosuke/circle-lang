cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(
	-pedantic-errors
	-Wall
	-Wextra
	-Wno-braced-scalar-init
	-Wno-sign-compare
)

set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -fsanitize=address,undefined")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -static")

project(circle-lang LANGUAGES CXX)

include_directories("./")


file(GLOB_RECURSE vendor CONFIGURE_DEPENDS "vendor/*.cpp")
add_library(vendor STATIC ${vendor})


file(GLOB_RECURSE lib CONFIGURE_DEPENDS "lib/*.cpp")
add_library(lib STATIC ${lib})
target_link_libraries(lib PRIVATE vendor)

find_package(tl-expected CONFIG REQUIRED)
target_link_libraries(lib PRIVATE tl::expected)

find_package(Boost CONFIG REQUIRED)
target_link_libraries(lib PRIVATE Boost::boost)


file(GLOB_RECURSE src CONFIGURE_DEPENDS "src/*.cpp")
add_executable(circle-lang ${src})
target_link_libraries(circle-lang PRIVATE lib)

find_package(boost_program_options CONFIG REQUIRED program_options)
target_link_libraries(circle-lang PRIVATE Boost::program_options)


enable_testing()

file(GLOB_RECURSE all-tests CONFIGURE_DEPENDS "test/*.cpp")
add_executable(all-tests ${all-tests})
target_link_libraries(all-tests PRIVATE lib)

find_package(GTest CONFIG REQUIRED)
target_link_libraries(all-tests PRIVATE GTest::gtest_main)


include(GoogleTest)
gtest_discover_tests(all-tests)
