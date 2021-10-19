# glfw 3.3
set(GLFW_LIB_NAME "GLFW")
set(GLFW_INC_PATH ${GLFW_LIB_NAME}/include)

set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)

add_subdirectory(${GLFW_LIB_NAME})

# glad
set(GLAD_LIB_NAME "glad")
set(GLAD_INC_PATH "${GLAD_LIB_NAME}/include")

add_library(glad STATIC "${GLAD_LIB_NAME}/src/glad.c")

target_include_directories(glad PUBLIC "${GLAD_INC_PATH}")

# imgui 1.85
set(IMGUI_LIB_NAME "imgui")
set(IMGUI_INC_PATH "${IMGUI_LIB_NAME}/")

set(IMGUI_SOURCES
    "${IMGUI_LIB_NAME}/imgui.cpp"
    "${IMGUI_LIB_NAME}/imgui_demo.cpp"
    "${IMGUI_LIB_NAME}/imgui_draw.cpp"
    "${IMGUI_LIB_NAME}/imgui_tables.cpp"
    "${IMGUI_LIB_NAME}/imgui_widgets.cpp"
    "${IMGUI_LIB_NAME}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_LIB_NAME}/backends/imgui_impl_opengl2.cpp"
)

set(IMGUI_HEADERS
    "${IMGUI_LIB_NAME}/imconfig.h"
    "${IMGUI_LIB_NAME}/imgui.h"
    "${IMGUI_LIB_NAME}/imgui_internal.h"
    "${IMGUI_LIB_NAME}/imstb_rectpack.h"
    "${IMGUI_LIB_NAME}/imstb_textedit.h"
    "${IMGUI_LIB_NAME}/imstb_truetype.h"
    "${IMGUI_LIB_NAME}/backends/imgui_impl_glfw.h"
    "${IMGUI_LIB_NAME}/backends/imgui_impl_opengl2.h"
)

add_library(${IMGUI_LIB_NAME}
    STATIC
        ${IMGUI_SOURCES}
        ${IMGUI_HEADERS}
)

target_compile_definitions(${IMGUI_LIB_NAME}
    PRIVATE
        IMGUI_IMPL_OPENGL_LOADER_GLAD=1
)

target_include_directories(${IMGUI_LIB_NAME}
    PUBLIC
        "${GLFW_INC_PATH}"
        "${GLAD_INC_PATH}"
        "${IMGUI_INC_PATH}"
)