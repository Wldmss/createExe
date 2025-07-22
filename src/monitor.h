// monitor.h
#ifndef MONITOR_H
#define MONITOR_H

#include <windows.h>

BOOL IsDualMonitorConnected(bool isFirst);
void MonitorForegroundWindow();

extern bool g_noAlert;
extern HWND g_mainWindow;
extern HWND g_headerWindow;

#endif
