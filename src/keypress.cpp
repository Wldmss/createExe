// keypress.cpp
#include "keypress.h"

#include <windows.h>
#include <stdio.h>

HHOOK g_hHook = NULL;

void ShowWarningMessage()
{
    // 기존 경고창 닫기
    HWND oldMsgBox = FindWindow(NULL, L"경고");
    if (oldMsgBox) {
        PostMessage(oldMsgBox, WM_CLOSE, 0, 0);
        Sleep(100);
    }

    if(!g_passKeyHook)
        MessageBoxW(NULL,
            L"평가 중 alt, ctrl 키와 같은 특수키를 사용하실 수 없습니다.\n반복적으로 사용하실 경우 평가에 불이익이 있을 수 있습니다.",
            L"경고",
            MB_OK | MB_ICONWARNING | MB_TOPMOST);
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && !g_passKeyHook)
    {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
        bool isKeyBlocked  = false;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
        {
            if ((p->flags & LLKHF_ALTDOWN) && p->vkCode == VK_TAB)  // Alt+Tab
            {
                isKeyBlocked = true;
            } else if (p->vkCode == VK_ESCAPE && (GetAsyncKeyState(VK_CONTROL) & 0x8000))   // Ctrl+Esc
            {
                isKeyBlocked = true;
            } else if (p->vkCode == VK_LWIN || p->vkCode == VK_RWIN || p->vkCode == VK_MENU || p->vkCode == VK_LMENU || p->vkCode == VK_RMENU)    // Windows 키, Alt 키
            {
                isKeyBlocked = true;
            } else if (p->vkCode == VK_CONTROL || p->vkCode == VK_LCONTROL || p->vkCode == VK_RCONTROL || p->vkCode == VK_F11 || p->vkCode == VK_F12)    // Ctrl 키, F11, F12
            {
                isKeyBlocked = true;
            }
        }

        if (isKeyBlocked)
        {
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ShowWarningMessage, NULL, 0, NULL);

            if(g_mainWindow) { 
                PostMessage(g_mainWindow, WM_RESTORE_FOCUS, 0, 0);
            }

            return 1; // 키 입력 무시
        }
    }

    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

void SetKeyboardHook()
{
    g_hHook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
}

void RemoveKeyboardHook()
{
    if (g_hHook)
    {
        UnhookWindowsHookEx(g_hHook);
        g_hHook = NULL;
    }
}
