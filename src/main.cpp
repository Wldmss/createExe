// main.cpp
#include <windows.h>

#include "view.h"
#include "monitor.h"
#include "keypress.h"

static HWND g_exitButton = nullptr;
HWND g_mainWindow = nullptr;
bool g_isExit = false;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DISPLAYCHANGE:
        {
            if (IsDualMonitorConnected(false))
            {
                // MessageBox(hwnd, L"듀얼 모니터가 연결되었습니다.", L"알림", MB_OK | MB_ICONINFORMATION);
                // 예: DestroyWindow(hwnd);
            }
        }
        return 0;

    case WM_ACTIVATEAPP:
        if (wParam == FALSE)
        {
            // MessageBox(hwnd, L"다른 창이 감지되었습니다.", L"경고", MB_OK | MB_ICONWARNING);
            // 또는: DestroyWindow(hwnd);
        }
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_EXIT_BUTTON)
        {
            g_isExit = true;
            int result = MessageBox(hwnd, L"시험을 종료하시겠습니까?", L"확인", MB_OKCANCEL | MB_ICONQUESTION);
            if (result == IDOK)
            {
                DestroyWindow(hwnd); // 확인 시 종료
            } else {
                g_isExit = false;
            }
        }
        return 0;

    case WM_SIZE:
        ResizeWebView(); // WebView 크기 조정
        if (g_exitButton)
        {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            MoveWindow(g_exitButton, width - 120, height - 50, 100, 30, TRUE);
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
    const wchar_t CLASS_NAME[] = L"MyWindowClass";

    WNDCLASS wc = { };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

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

    bool isTest = true;    // 테스트 시 true

    if (!IsDualMonitorConnected(true) || isTest)
    {
         // exit button
        g_exitButton = CreateExitButton(hwnd, hInst);

        // key hook
        SetKeyboardHook();

        // other window
        g_mainWindow = hwnd;
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MonitorForegroundWindow, NULL, 0, NULL);

        // open webview
        LaunchWebView(hwnd);
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
