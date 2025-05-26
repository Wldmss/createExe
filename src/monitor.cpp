#include "monitor.h"

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

extern bool g_isExit;

// ���� ���â Ȯ��
void closeOldMsgBox(LPCWSTR msg)
{
    
    HWND oldMsgBox = FindWindow(NULL, L"���");
    if (!oldMsgBox) {
        MessageBox(NULL, msg, L"���", MB_OK | MB_ICONWARNING | MB_TOPMOST);
    }
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    int* monitorCount = (int*)dwData;
    (*monitorCount)++;
    return TRUE;
}

// ��� ����� Ȯ��
BOOL IsDualMonitorConnected(bool isFirst)
{
    int monitorCount = 0;

    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&monitorCount);

    bool isDualMonitor = monitorCount >= 2;

    if(isDualMonitor && !g_isExit) {
        closeOldMsgBox(L"���� ����� ������ �������ּ���." + (!isFirst && L" ���� ����Ͱ� ����Ǿ� �ִ� ��� �򰡿� �������� ���� �� �ֽ��ϴ�."));
    }

    return isDualMonitor;
}

extern HWND g_mainWindow;

// �ٸ� â ����
void MonitorForegroundWindow()
{
    while (true)
    {
        HWND foreground = GetForegroundWindow();
        if (foreground && foreground != g_mainWindow)
        {
            
            if(!g_isExit) closeOldMsgBox(L"�ٸ� â�� �����Ǿ����ϴ�. �ٸ� ȭ���� �ݺ������� ����Ǵ� ��� �򰡿� �������� ���� �� �ֽ��ϴ�.");

            // �ʿ� �� ����
            // PostMessage(g_mainWindow, WM_CLOSE, 0, 0);
        }

        Sleep(1000); // 1�ʸ��� �˻�
    }
}
