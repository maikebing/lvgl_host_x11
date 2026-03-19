# LVGL X11 Host Library

A lightweight X11 host for running LVGL examples or applications on Linux desktops.
一个轻量的 X11 宿主实现，用来在 Linux 桌面环境运行 LVGL 示例或应用。

## Highlights / 当前改进点

- More reliable window-close handling via `WM_DELETE_WINDOW`
- Avoids repeated LVGL initialization by checking `lv_is_initialized()`
- Adds keyboard and mouse-wheel support
- Adds basic LVGL v8 / v9 compatibility
- 通过 `WM_DELETE_WINDOW` 更稳地处理窗口关闭
- 通过 `lv_is_initialized()` 避免重复初始化 LVGL
- 增加键盘和鼠标滚轮支持
- 对 LVGL v8 / v9 做了基础兼容

## Dependencies / 依赖

Required build dependencies:

- CMake
- GCC / Clang
- X11 development headers/libraries, e.g. `libx11-dev` on Debian/Ubuntu
- LVGL development files discoverable via `pkg-config --modversion lvgl`

当前仓库在构建时仍依赖这些系统开发包。
If `libx11-dev` or `lvgl.pc` is missing, CMake will fail during configuration.

## Compatibility notes / 兼容性说明

- Basic LVGL v8 / v9 API differences are handled in the host implementation
- `lvgl_host_x11_init()` only calls `lv_init()` when LVGL is not already initialized
- The host still assumes a 32-bit LVGL pixel buffer, so `LV_COLOR_DEPTH=32` is required
- The window currently keeps the fixed size passed at initialization; resizing by dragging the window border is not supported, so the X11 window may resize while LVGL rendering still uses the original buffer size, causing visual artifacts
- 已处理 LVGL v8 / v9 的基础 API 差异
- `lvgl_host_x11_init()` 只会在 LVGL 尚未初始化时调用 `lv_init()`
- 当前仍假定 LVGL 输出为 32-bit 像素缓冲，因此需要 `LV_COLOR_DEPTH=32`
- 当前窗口按初始化尺寸固定使用，不支持通过拖拽窗口边框改变大小；如果强行调整窗口，X11 窗口尺寸可能变化，但 LVGL 仍按原始缓冲区绘制，从而出现显示异常

## Input support / 输入支持

- Mouse move / left click
- Arrow keys, Enter, Esc, Backspace, Home, End, Delete, Tab / Shift+Tab
- Mouse wheel mapped to LVGL encoder input
- 鼠标移动 / 左键点击
- 键盘方向键、Enter、Esc、Backspace、Home、End、Delete、Tab / Shift+Tab
- 鼠标滚轮（映射为 LVGL encoder 输入）

## Build / 构建

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

## Run the example / 运行示例

```bash
./lvgl_host_x11_example
```

## Example usage / 示例用法

```c
lvgl_host_x11_t host;
if (lvgl_host_x11_init(&host, 800, 480, "LVGL X11 Host Example") != 0) {
    return 1;
}
```
