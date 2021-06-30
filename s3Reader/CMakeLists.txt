cmake_minimum_required(VERSION 3.19)

# Set the project name and version
project(s3Reader VERSION 0.1)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(S3READER_FILES
        src/GameHandler.cpp
        src/S3Stats.cpp)

# add_executable(s3Reader src/main.cpp)
add_executable(${PROJECT_NAME} src/main.cpp ${S3READER_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(include)