#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <atomic>
#include <string>

#pragma comment(lib, "psapi.lib")

namespace {
    // 全局标志用于优雅退出
    std::atomic<bool> g_isRunning{true};
    HANDLE g_hMutex = nullptr;
    HWND g_hWnd = nullptr;
    
    // 窗口过程（用于接收消息）
    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        if (message == WM_CLOSE) {
            g_isRunning = false;
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    
    // 创建隐藏窗口用于接收消息
    bool CreateMessageWindow() {
        WNDCLASSW wc = {0};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = L"WinNoSleepTool_MsgWindow";
        RegisterClassW(&wc);
        
        g_hWnd = CreateWindowExW(0, L"WinNoSleepTool_MsgWindow", L"",
                                  WS_OVERLAPPED, 0, 0, 0, 0,
                                  HWND_MESSAGE, nullptr, GetModuleHandleW(nullptr), nullptr);
        return g_hWnd != nullptr;
    }

    // 获取当前程序的可执行文件路径
    std::wstring GetCurrentExecutablePath() {
        wchar_t path[MAX_PATH] = {0};
        GetModuleFileNameW(nullptr, path, MAX_PATH);
        return std::wstring(path);
    }

    // 检查进程是否为同一程序（通过路径匹配，防止误杀其他程序）
    bool IsSameProgram(DWORD processId, const std::wstring& currentPath) {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE,
                                      processId);
        if (!hProcess) {
            return false;
        }

        wchar_t processPath[MAX_PATH] = {0};
        if (GetModuleFileNameExW(hProcess, nullptr, processPath, MAX_PATH) == 0) {
            CloseHandle(hProcess);
            return false;
        }
        CloseHandle(hProcess);

        // 不区分大小写比较路径
        return _wcsicmp(processPath, currentPath.c_str()) == 0;
    }

    // 终止指定的进程
    bool TerminateProcessById(DWORD processId) {
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, processId);
        if (!hProcess) {
            return false;
        }

        bool result = TerminateProcess(hProcess, 0) != FALSE;

        // 等待进程退出（最多2秒）
        if (result) {
            WaitForSingleObject(hProcess, 2000);
        }

        CloseHandle(hProcess);
        return result;
    }

    // 查找已运行实例的窗口句柄
    HWND FindExistingWindow() {
        return FindWindowW(L"WinNoSleepTool_MsgWindow", nullptr);
    }

    // 查找并停止已运行的实例
    bool StopExistingInstance(const std::wstring& currentPath) {
        // 先尝试通过窗口消息优雅退出
        HWND hWnd = FindExistingWindow();
        if (hWnd) {
            SendMessageW(hWnd, WM_CLOSE, 0, 0);
            Sleep(1000);  // 等待进程优雅退出
            // 检查进程是否还在运行
            hWnd = FindExistingWindow();
            if (!hWnd) {
                return true;  // 已成功退出
            }
        }

        // 如果优雅退出失败，使用强制终止
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) {
            return false;
        }

        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);

        if (!Process32FirstW(hSnapshot, &pe32)) {
            CloseHandle(hSnapshot);
            return false;
        }

        DWORD currentPid = GetCurrentProcessId();
        bool found = false;

        do {
            // 跳过当前进程
            if (pe32.th32ProcessID == currentPid) {
                continue;
            }

            // 检查是否为同一程序
            if (IsSameProgram(pe32.th32ProcessID, currentPath)) {
                found = true;
                TerminateProcessById(pe32.th32ProcessID);
                break;
            }
        } while (Process32NextW(hSnapshot, &pe32));

        CloseHandle(hSnapshot);
        return found;
    }

    // 信号处理函数
    BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType) {
        switch (dwCtrlType) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
            g_isRunning = false;
            return TRUE;
        default:
            return FALSE;
        }
    }
}

int main() {
    std::wstring currentPath = GetCurrentExecutablePath();

    // 使用固定互斥锁名称（防止误杀通过路径匹配实现）
    g_hMutex = CreateMutexW(nullptr, TRUE, L"WinNoSleepTool_Mutex");

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // 互斥锁已存在，说明程序已在运行 -> 停止旧实例，新实例也退出
        if (g_hMutex) {
            CloseHandle(g_hMutex);
        }

        // 通过路径匹配找到并停止已运行的实例（避免误杀其他程序）
        StopExistingInstance(currentPath);
        
        // 显示退出提示
        MessageBoxW(nullptr, L"防休眠工具已退出", L"WinNoSleepTool",
                   MB_OK | MB_ICONINFORMATION);
        return 0;
    }

    // 创建隐藏窗口用于接收消息
    if (!CreateMessageWindow()) {
        if (g_hMutex) {
            CloseHandle(g_hMutex);
        }
        return 1;
    }

    // 设置控制台控制处理程序
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

    // 设置线程执行状态，防止系统休眠和显示器关闭
    SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);

    // 消息循环
    MSG msg;
    while (g_isRunning.load()) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            if (!g_isRunning.load()) {
                break;
            }
        }
        
        if (g_isRunning.load()) {
            SleepEx(100, FALSE);
        }
    }

    // 恢复系统默认休眠行为
    SetThreadExecutionState(ES_CONTINUOUS);

    // 清理资源
    if (g_hWnd) {
        DestroyWindow(g_hWnd);
    }
    if (g_hMutex) {
        ReleaseMutex(g_hMutex);
        CloseHandle(g_hMutex);
    }

    return 0;
}