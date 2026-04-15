cmake_minimum_required(VERSION 3.24)

project(maplibre-native-godot-tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_executable(test_animation test_animation.cpp)
target_include_directories(test_animation PRIVATE "${CMAKE_SOURCE_DIR}/../src")

add_test(NAME animation COMMAND test_animation)
