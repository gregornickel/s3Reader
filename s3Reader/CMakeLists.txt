cmake_minimum_required(VERSION 3.19)

# Set PROJECT_NAME and version
project(s3Reader VERSION 0.1)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set compiler flags
set (CMAKE_CXX_FLAGS "/EHa")

# ---------- Add thrid party libraries ----------
# nlohmann json 3.10.0
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(external/json)

# boost 1.77.0
if(WIN32)
    set(BOOST_ROOT ${CMAKE_HOME_DIRECTORY}/external/boost)
endif ()
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "external/boost/boost")
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "external/boost/lib")
# set(Boost_USE_STATIC_LIBS OFF)
# set(Boost_USE_MULTITHREADED ON)
# set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.77.0 REQUIRED COMPONENTS) # header only libraries do not need to be added here
include_directories(${Boost_INCLUDE_DIRS})
# -----------------------------------------------

# Include header files
include_directories(include)

set(FILES_S3READER
    src/main.cpp
    src/GameHandler.cpp
    src/S3Stats.cpp
    src/WebSocketServer.cpp
    src/Listener.cpp
    src/WebSocketSession.cpp
    src/SharedState.cpp
)

add_executable(${PROJECT_NAME} ${FILES_S3READER})

target_link_libraries(${PROJECT_NAME}
    PRIVATE nlohmann_json::nlohmann_json
    PRIVATE ${Boost_LIBRARIES}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
