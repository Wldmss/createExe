#include "browser.h"
#include <tlhelp32.h>
#include <stdio.h>

HWND g_chromeWindow = NULL;
DWORD g_chromePID = 0;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM)
{
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == g_chromePID)
    {
        g_chromeWindow = hwnd;
        return FALSE;
    }
    return TRUE;
}

bool OpenChromeKioskAndTrackWindow()
{
    const wchar_t* chromePath = L"C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe";
    const wchar_t* args = L"--kiosk https://naver.com";
    wchar_t  cmdLine[512];
    swprintf_s(cmdLine, _countof(cmdLine), L"\"%s\" %s", chromePath, args);

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = {};

    if (!CreateProcessW(nullptr, cmdLine, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        return false;
    }

    g_chromePID = pi.dwProcessId;
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    for (int i = 0; i < 20; ++i)
    {
        EnumWindows(EnumWindowsProc, 0);
        if (g_chromeWindow) break;
        Sleep(500);
    }

    return g_chromeWindow != NULL;
}

DWORD WINAPI MonitorThread(LPVOID)
{
    while (true)
    {
        HWND fg = GetForegroundWindow();
        if (fg != g_chromeWindow)
        {
            MessageBoxW(NULL, L"다른 창으로 전환이 감지되었습니다!", L"경고", MB_OK | MB_TOPMOST);
        }
        Sleep(1000);
    }
    return 0;
}

void StartBrowserMonitor()
{
    CreateThread(NULL, 0, MonitorThread, NULL, 0, NULL);
}

void OpenWebPage()
{
    ShellExecuteW(NULL, L"open", L"https://ktedu.kt.com", NULL, NULL, SW_SHOWNORMAL);
}