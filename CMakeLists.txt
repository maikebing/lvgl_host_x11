cmake_minimum_required(VERSION 3.16)

project(lvgl_host_x11 C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
find_package(X11 REQUIRED)

pkg_check_modules(LVGL REQUIRED lvgl)

add_library(lvgl_host_x11
    src/lvgl_host_x11.c
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

target_link_directories(lvgl_host_x11
    PRIVATE
        ${LVGL_LIBRARY_DIRS}
)

target_link_libraries(lvgl_host_x11
    PUBLIC
        X11::X11
        ${LVGL_LIBRARIES}
)

add_executable(lvgl_host_x11_example
    examples/main.c
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

target_link_directories(lvgl_host_x11_example
    PRIVATE
        ${LVGL_LIBRARY_DIRS}
)

target_link_libraries(lvgl_host_x11_example
    PRIVATE
        lvgl_host_x11
        X11::X11
        ${LVGL_LIBRARIES}
)