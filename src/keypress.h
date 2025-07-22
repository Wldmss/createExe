// keypress.h
#ifndef KEYPRESS_H
#define KEYPRESS_H

#include <windows.h> 

#define WM_RESTORE_FOCUS (WM_APP + 1)

extern bool g_passKeyHook;
extern HWND g_mainWindow;

void SetKeyboardHook();
void RemoveKeyboardHook();

#endif
