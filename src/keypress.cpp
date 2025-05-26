// keypress.cpp
#include "keypress.h"

#include <windows.h>
#include <stdio.h>

HHOOK g_hHook = NULL;
extern bool g_isExit;

void ShowWarningMessage()
{
    // ���� ���â �ݱ�
    HWND oldMsgBox = FindWindow(NULL, L"���");
    if (oldMsgBox) {
        PostMessage(oldMsgBox, WM_CLOSE, 0, 0);
        Sleep(100);
    }

    if(!g_isExit)
        MessageBoxW(NULL,
            L"�� �� alt, ctrl Ű�� ���� Ư��Ű�� ����Ͻ� �� �����ϴ�.\n�ݺ������� ����Ͻ� ��� �򰡿� �������� ���� �� �ֽ��ϴ�.",
            L"���",
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

        if (isKeyBlocked)
        {
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ShowWarningMessage, NULL, 0, NULL);
            return 1; // Ű �Է� ����
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
