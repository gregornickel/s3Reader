cmake_minimum_required(VERSION 3.19)

# Set PROJECT_NAME and version
project(s3Reader VERSION 0.1)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(FILES_S3READER
    src/main.cpp
    src/GameHandler.cpp
    src/S3Stats.cpp
)

include_directories(include)

add_executable(${PROJECT_NAME} ${FILES_S3READER})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
