# WinNoSleepTool

一个轻量级的 Windows 防休眠工具，防止系统进入休眠状态并保持显示器开启。

## 功能特性

- 🚀 **轻量级**: 不使用 iostream，仅依赖 Windows API，体积小、资源占用低
- 🔒 **防休眠**: 防止 Windows 系统进入休眠状态
- 💻 **保持显示**: 防止显示器关闭
- 🎯 **开关模式**: 双击启动，再次双击停止（退出时显示提示）
- 🔐 **单实例**: 自动检测并管理单实例运行
- 🛡️ **安全停止**: 通过路径匹配防止误杀其他程序
- ⚙️ **静态编译**: 无外部依赖，可直接运行

## 系统要求

- Windows 操作系统
- CMake 3.15+ 和 Ninja（用于编译）

## 编译

### 使用 CMake + Ninja

```bash
mkdir build
cd build
cmake -G Ninja ..
ninja
```

编译完成后，可执行文件位于 `build/WinNoSleepTool.exe`

### 使用 Makefile (MinGW)

```bash
make
```

## 使用方法

### 启动/停止

1. **第一次双击** `WinNoSleepTool.exe`：
   - 程序静默启动，后台运行
   - 系统将保持唤醒状态，显示器保持开启

2. **第二次双击** `WinNoSleepTool.exe`：
   - 停止已运行的实例
   - 显示消息框："防休眠工具已退出"
   - 点击确定后，新实例也退出

### 注意事项

- 程序运行时不显示任何窗口，完全后台运行
- 退出时会显示消息框提示
- 即使程序被强制终止，系统也会自动恢复休眠功能

## 工作原理

程序使用 Windows API 的 `SetThreadExecutionState` 函数，通过设置以下标志来防止系统休眠：

- `ES_CONTINUOUS`: 持续保持执行状态（直到程序退出）
- `ES_SYSTEM_REQUIRED`: 防止系统进入休眠状态
- `ES_DISPLAY_REQUIRED`: 防止显示器关闭

程序退出时会自动调用 `SetThreadExecutionState(ES_CONTINUOUS)` 清除请求，恢复系统默认休眠行为。

## 技术细节

- **单实例检测**: 使用互斥锁 `WinNoSleepTool_Mutex` 检测程序是否已运行
- **安全停止**: 通过完整路径匹配确保只停止同一程序的实例
- **优雅退出**: 优先通过窗口消息（WM_CLOSE）优雅退出，失败则强制终止
- **资源清理**: 程序退出时自动清理所有资源（互斥锁、窗口句柄等）

## 许可证

MIT License - 详见 [LICENSE](LICENSE) 文件

## 作者

Yundi339
