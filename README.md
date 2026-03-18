# LVGL X11 Host Library

## Build Prerequisites
- CMake
- gcc
- libX11-dev
- liblvgl-dev

## Project Structure

- include/
    - lvgl_host_x11.h
- src/
    - lvgl_host_x11.c
- examples/
    - main.c
- README.md
- .gitignore
- CMakeLists.txt

## Build and Run Commands
```bash
mkdir build
cd build
cmake ..
make
./example/main
```