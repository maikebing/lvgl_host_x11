cmake_minimum_required(VERSION 3.16)

project(lvgl_host_x11 C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
find_package(X11 REQUIRED)

# Prefer a bundled LVGL checkout when provided by the parent repo.
# Fallback to pkg-config for system-installed LVGL.
set(LVGL_BUNDLED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../lvgl" CACHE PATH "Path to bundled LVGL source tree")
set(LVGL_BUNDLED_LIB "" CACHE FILEPATH "Path to bundled liblvgl shared library")

if(NOT LVGL_BUNDLED_LIB)
    set(_LVGL_BUNDLED_LIB_CANDIDATES
        "${CMAKE_CURRENT_SOURCE_DIR}/../build/lvgl-x11/lib/liblvgl.so.9"
        "${CMAKE_CURRENT_SOURCE_DIR}/../build/lvgl-x11/lib/liblvgl.so"
        "${LVGL_BUNDLED_DIR}/build-shared/lib/liblvgl.so.9"
        "${LVGL_BUNDLED_DIR}/build-shared/lib/liblvgl.so"
    )

    foreach(_lvgl_candidate IN LISTS _LVGL_BUNDLED_LIB_CANDIDATES)
        if(EXISTS "${_lvgl_candidate}")
            set(LVGL_BUNDLED_LIB "${_lvgl_candidate}" CACHE FILEPATH "Path to bundled liblvgl shared library" FORCE)
            break()
        endif()
    endforeach()
endif()

set(LVGL_INCLUDE_DIRS "")
set(LVGL_LIBRARIES "")
set(LVGL_LIBRARY_DIRS "")
set(LVGL_CFLAGS_OTHER "")

if(EXISTS "${LVGL_BUNDLED_DIR}/lvgl.h" AND EXISTS "${LVGL_BUNDLED_LIB}")
    message(STATUS "Using bundled LVGL from ${LVGL_BUNDLED_DIR}")
    set(LVGL_INCLUDE_DIRS
        ${LVGL_BUNDLED_DIR}
        ${LVGL_BUNDLED_DIR}/src
    )
    set(LVGL_LIBRARIES ${LVGL_BUNDLED_LIB})
else()
    pkg_check_modules(LVGL REQUIRED lvgl)
endif()

add_library(lvgl_host_x11 SHARED
    src/lvgl_host_x11.c
)

set_target_properties(lvgl_host_x11 PROPERTIES
    BUILD_RPATH "$ORIGIN"
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "$ORIGIN"
    INSTALL_RPATH_USE_LINK_PATH FALSE
)

target_include_directories(lvgl_host_x11
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${LVGL_INCLUDE_DIRS}
)

target_compile_options(lvgl_host_x11
    PRIVATE
        ${LVGL_CFLAGS_OTHER}
)

if(LVGL_LIBRARY_DIRS)
    target_link_directories(lvgl_host_x11
        PRIVATE
            ${LVGL_LIBRARY_DIRS}
    )
endif()

target_link_libraries(lvgl_host_x11
    PUBLIC
        X11::X11
        ${LVGL_LIBRARIES}
)

add_executable(lvgl_host_x11_example
    examples/main.c
)

set_target_properties(lvgl_host_x11_example PROPERTIES
    BUILD_RPATH "$ORIGIN"
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "$ORIGIN"
    INSTALL_RPATH_USE_LINK_PATH FALSE
)

target_include_directories(lvgl_host_x11_example
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${LVGL_INCLUDE_DIRS}
)

target_compile_options(lvgl_host_x11_example
    PRIVATE
        ${LVGL_CFLAGS_OTHER}
)

if(LVGL_LIBRARY_DIRS)
    target_link_directories(lvgl_host_x11_example
        PRIVATE
            ${LVGL_LIBRARY_DIRS}
    )
endif()

target_link_libraries(lvgl_host_x11_example
    PRIVATE
        lvgl_host_x11
        X11::X11
        ${LVGL_LIBRARIES}
)
