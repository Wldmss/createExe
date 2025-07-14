// main.cpp
#include <windows.h>

#include "view.h"
#include "monitor.h"
#include "keypress.h"

bool g_noAlert = true;   // 다른 창 경고 표시 여부 (TEST 용)
bool g_isPass = true;    // key hook, 듀얼 모니터 체크 pass (TEST 용)

HWND g_mainWindow = nullptr;

const wchar_t CLASS_NAME[] = L"MyWindowClass";
const wchar_t HEADER_CLASS_NAME[] = L"CustomHeaderClass";

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DISPLAYCHANGE:
        IsDualMonitorConnected(false);
        return 0;

    case WM_ACTIVATEAPP:
        if (wParam == FALSE)
        {
            // MessageBox(hwnd, L"다른 창이 감지되었습니다.", L"경고", MB_OK | MB_ICONWARNING);
            // 또는: DestroyWindow(hwnd);
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Main
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
    SetConsoleOutputCP(949); // EUC-KR (CP949)

    // 메인 윈도우 클래스
    WNDCLASS wc = { };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    // 헤더 윈도우 클래스
    WNDCLASS wcHeader = { };
    wcHeader.lpfnWndProc = HeaderWindowProc;
    wcHeader.hInstance = hInst;
    wcHeader.lpszClassName = HEADER_CLASS_NAME;
    RegisterClass(&wcHeader);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"지니어스 대량평가",
        WS_POPUP,
        0, 0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        nullptr,
        nullptr,
        hInst,
        nullptr
    );

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    if (!IsDualMonitorConnected(true) || g_isPass)
    {
        // key hook
        SetKeyboardHook();

        // other window
        g_mainWindow = hwnd;
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MonitorForegroundWindow, NULL, 0, NULL);

        // open webview
        LaunchWebView(hwnd, hInst);
    } else {
        DestroyWindow(hwnd);
    }

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    RemoveKeyboardHook();

    return 0;
}
