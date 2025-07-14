#include "monitor.h"

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

extern bool g_noAlert
;

// 기존 경고창 확인
void closeOldMsgBox(LPCWSTR msg)
{
    
    HWND oldMsgBox = FindWindow(NULL, L"경고");
    if (!oldMsgBox && !g_noAlert) {
        MessageBox(NULL, msg, L"경고", MB_OK | MB_ICONWARNING | MB_TOPMOST);
    }
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    int* monitorCount = (int*)dwData;
    (*monitorCount)++;
    return TRUE;
}

// 듀얼 모니터 확인
BOOL IsDualMonitorConnected(bool isFirst)
{
    int monitorCount = 0;

    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&monitorCount);

    bool isDualMonitor = monitorCount >= 2;

    if(isDualMonitor) {
        closeOldMsgBox(L"보조 모니터 연결을 해제해주세요." + (!isFirst && L" 보조 모니터가 연결되어 있는 경우 평가에 불이익이 있을 수 있습니다."));
    }

    return isDualMonitor;
}

extern HWND g_mainWindow;
extern HWND g_headerWindow;

// 다른 창 감지
void MonitorForegroundWindow()
{
    while (true)
    {
        HWND foreground = GetForegroundWindow();
        if (foreground && foreground != g_mainWindow && foreground != g_headerWindow)
        {
            
            closeOldMsgBox(L"다른 창이 감지되었습니다. 다른 화면이 반복적으로 실행되는 경우 평가에 불이익이 있을 수 있습니다.");

            // 필요 시 종료
            // PostMessage(g_mainWindow, WM_CLOSE, 0, 0);
        }

        Sleep(1000); // 1초마다 검사
    }
}
