cmake_minimum_required(VERSION 3.14)
project(valkey_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(valkey main.cpp)

add_subdirectory(lib)
enable_testing()
add_subdirectory(tests)

target_link_libraries(valkey PRIVATE valkey_lib)
target_include_directories(valkey PRIVATE lib/src)
