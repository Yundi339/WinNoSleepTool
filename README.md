# WinNoSleepTool

一个轻量级的 Windows 防休眠工具，防止系统进入休眠状态并保持显示器开启。

## 功能特性

- 🚀 **轻量级**: 不使用 iostream，仅依赖 Windows API，体积小、资源占用低
- 🔒 **防休眠**: 防止 Windows 系统进入休眠状态
- 💻 **保持显示**: 防止显示器关闭
- 🎯 **优雅退出**: 支持 Ctrl+C 优雅退出，自动恢复系统默认休眠设置
- ⚡ **自动刷新**: 每3分钟自动刷新执行状态，确保长期运行稳定

## 系统要求

- Windows 操作系统
- MinGW-w64 或 Visual Studio 编译器

## 编译

### 使用 Makefile (MinGW)

```bash
make
```

或者单独编译发布版本：

```bash
make release
```

编译调试版本：

```bash
make debug
```

清理编译文件：

```bash
make clean
```

## 使用方法

1. 运行编译生成的 `win_no_sleep.exe`
2. 程序将防止系统休眠和显示器关闭
3. 按 `Ctrl+C` 退出程序，系统将恢复默认休眠设置

## 工作原理

程序使用 Windows API 的 `SetThreadExecutionState` 函数，通过设置以下标志来防止系统休眠：

- `ES_CONTINUOUS`: 持续保持执行状态
- `ES_SYSTEM_REQUIRED`: 防止系统进入休眠状态
- `ES_DISPLAY_REQUIRED`: 防止显示器关闭

程序每3分钟自动刷新执行状态，确保长期运行的稳定性。

## 许可证

MIT License - 详见 [LICENSE](LICENSE) 文件

## 作者

Yundi339
