cmake_minimum_required(VERSION 3.24)
project(injector)

set(CMAKE_CXX_STANDARD 17)
set(Boost_USE_STATIC_LIBS ON)

find_package(Boost 1.80 REQUIRED COMPONENTS program_options)

message(DEBUG ${Boost_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

add_executable(injector main.cpp simple_windows.h find_process_windows.h)
target_link_libraries(injector Boost::program_options)