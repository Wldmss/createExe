// keypress.cpp
#include "keypress.h"

#include <windows.h>
#include <stdio.h>

HHOOK g_hHook = NULL;
extern bool g_isPass;

void ShowWarningMessage()
{
    // 기존 경고창 닫기
    HWND oldMsgBox = FindWindow(NULL, L"경고");
    if (oldMsgBox) {
        PostMessage(oldMsgBox, WM_CLOSE, 0, 0);
        Sleep(100);
    }

    if(!g_isPass)
        MessageBoxW(NULL,
            L"평가 중 alt, ctrl 키와 같은 특수키를 사용하실 수 없습니다.\n반복적으로 사용하실 경우 평가에 불이익이 있을 수 있습니다.",
            L"경고",
            MB_OK | MB_ICONWARNING | MB_TOPMOST);
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
        bool isKeyBlocked  = false;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
        {
            switch (p->vkCode)
            {
                case VK_CONTROL:      // Ctrl
                case VK_RCONTROL:
                case VK_LCONTROL:
                case VK_MENU:         // Alt
                case VK_RMENU:
                case VK_LMENU:
                case VK_F11:
                case VK_F12:
                case VK_LWIN:           // Windows
                case VK_RWIN:
                    isKeyBlocked = true;
                    break;

                default:
                    break;
            }

            // Alt+Tab
            if ((GetAsyncKeyState(VK_MENU) & 0x8000) && p->vkCode == VK_TAB)
            {
                isKeyBlocked = true;
            }

            // Ctrl + Esc
            if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && p->vkCode == VK_ESCAPE)
            {
                isKeyBlocked = true;
            }
        }

        if (isKeyBlocked && !g_isPass)
        {
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ShowWarningMessage, NULL, 0, NULL);
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
