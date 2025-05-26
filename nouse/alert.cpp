// alert.cpp
#include "alert.h"

HWND g_hwnd = NULL;

void InitAndShowUI(HINSTANCE hInstance, int nCmdShow)
{
    // const wchar_t CLASS_NAME[] = L"MyWindowClass";

    // WNDCLASSW wc = {};
    // wc.lpfnWndProc = WndProc;
    // wc.hInstance = hInstance;
    // wc.lpszClassName = CLASS_NAME;

    // RegisterClassW(&wc);

    // g_hwnd = CreateWindowExW(
    //     0, CLASS_NAME, L"���Ͼ �뷮��",
    //     WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
    //     NULL, NULL, hInstance, NULL);

    // CreateWindowW(L"BUTTON", L"�� ����",
    //     WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
    //     100, 100, 200, 50,
    //     g_hwnd, (HMENU)ID_EXIT_BUTTON, hInstance, NULL);

    // ShowWindow(g_hwnd, nCmdShow);
}
