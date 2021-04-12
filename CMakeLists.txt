cmake_minimum_required(VERSION 3.19)

# Set the project name and version
project(s3Reader VERSION 0.1)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(s3Reader src/main.cpp)

target_include_directories(s3Reader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
