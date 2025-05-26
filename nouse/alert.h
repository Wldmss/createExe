#ifndef ALERT_H
#define ALERT_H

#include <windows.h>

#define ID_EXIT_BUTTON 1  // 평가 종료 버튼 ID

extern HWND g_hwnd;  // UI 핸들 외부 접근용

void InitAndShowUI(HINSTANCE hInstance, int nCmdShow);

#endif
