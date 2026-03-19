# LVGL X11 Host Library

一个轻量的 X11 宿主实现，用来在 Linux 桌面环境运行 LVGL 示例或应用。

## 当前改进点

- 更稳的窗口关闭处理：支持窗口管理器发送的 `WM_DELETE_WINDOW`
- 避免库内部重复执行 `lv_init()`：如果应用已经初始化 LVGL，不会再次初始化
- 增加键盘输入与鼠标滚轮支持
- 兼容更多 LVGL 版本：对 LVGL v8 / v9 做了基础适配

## 依赖

构建依赖以下开发包：

- CMake
- GCC / Clang
- X11 开发文件（如 Debian/Ubuntu 上的 `libx11-dev`）
- LVGL 开发文件，并且需要能被 `pkg-config --modversion lvgl` 找到

> 注意：当前环境如果缺少 `libx11-dev` 或 `lvgl.pc`，CMake 会在配置阶段失败。

## 兼容性说明

- 已对 LVGL v8 / v9 的核心 API 差异做兼容处理
- 示例代码会显式先调用 `lv_init()`
- 宿主实现仍然假定 LVGL 输出为 32-bit 像素缓冲；请使用 `LV_COLOR_DEPTH=32`
- 目前窗口大小固定按初始化传入尺寸使用，未额外实现运行时重建缓冲区

## 输入支持

- 鼠标移动 / 左键点击
- 键盘方向键、Enter、Esc、Backspace、Home、End、Delete、Tab / Shift+Tab
- 鼠标滚轮（映射为 LVGL encoder 输入）

## 构建

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

## 运行示例

```bash
./lvgl_host_x11_example
```

## 示例代码要点

应用侧推荐自行负责 LVGL 初始化：

```c
lv_init();

lvgl_host_x11_t host;
if (lvgl_host_x11_init(&host, 800, 480, "LVGL X11 Host Example") != 0) {
    return 1;
}
```
