#ifndef VIEW_H
#define VIEW_H

#include <windows.h>

LRESULT CALLBACK HeaderWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void LaunchWebView(HWND parent, HINSTANCE hInst);

#define ID_EXIT_BUTTON 1001
#define ID_EXEC_PPT 1002
#define HEADER_HEIGHT 50
#define ID_HEADER 2001
#define WM_RESTORE_FOCUS (WM_APP + 1)
#define WM_HIDETASKBAR (WM_APP + 2)
#define WM_SHOWTASKBAR (WM_APP + 3)

// 전역 변수 extern 선언
extern HWND g_headerWindow;
extern HWND g_exitButton;
extern HWND g_mainWindow;

extern const wchar_t MAIN_CLASS_NAME[];
extern const wchar_t HEADER_CLASS_NAME[];

#endif