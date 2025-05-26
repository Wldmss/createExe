#ifndef VIEW_H
#define VIEW_H

#include <windows.h>

#define ID_EXIT_BUTTON 1001

void ResizeWebView();
HWND CreateExitButton(HWND hwnd, HINSTANCE hInst);
void LaunchWebView(HWND parentWindow);

#endif