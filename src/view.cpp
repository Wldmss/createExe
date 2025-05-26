// view.cpp
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "WebView2LoaderStatic.lib") // WebView2 정적 링크 시

#include <windows.foundation.h>
#include <wrl.h>
#include <wrl/event.h>
#include <wil/com.h> 

#include <windows.h>
#include <WebView2.h>
#include "view.h"
#include <string>

#include <shlobj.h>
#include <filesystem>

using namespace Microsoft::WRL;

// 전역 변수로 WebView2 객체들 유지 (view.cpp 내 전역)
static wil::com_ptr<ICoreWebView2Environment> g_webviewEnvironment;
static wil::com_ptr<ICoreWebView2Controller> g_webviewController;
static wil::com_ptr<ICoreWebView2> g_webviewWindow;

static HWND g_parentWindow = nullptr;
const wchar_t WEB_URL[] = L"https://ktedu.kt.com";

#define ID_EXIT_BUTTON 1001 // 버튼 ID

// 윈도우 크기 변경 시 WebView 크기 맞추기
void ResizeWebView()
{
    if (g_webviewController && g_parentWindow)
    {
        RECT bounds;
        GetClientRect(g_parentWindow, &bounds);
        g_webviewController->put_Bounds(bounds);
    }
}

// 평가 종료 버튼
HWND CreateExitButton(HWND hwnd, HINSTANCE hInst) {
    return CreateWindow(
        L"BUTTON", L"평가 종료",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        0, 0, 100, 30,
        hwnd,
        (HMENU)ID_EXIT_BUTTON,
        hInst,
        nullptr
    );
}

// webview load
void LaunchWebView(HWND parentWindow)
{
    g_parentWindow = parentWindow;

     // 1. 임시 폴더 경로 가져오기
    PWSTR tempPath = nullptr;
    std::wstring userDataFolder;

    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &tempPath)))
    {
        userDataFolder = std::wstring(tempPath) + L"\\Temp\\KT_Genius_Webview2";
        CoTaskMemFree(tempPath);
    }
    else
    {
        MessageBox(parentWindow, L"임시 폴더를 가져올 수 없습니다.", L"오류", MB_OK | MB_ICONERROR);
        return;
    }

    HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
        nullptr,
        userDataFolder.c_str(),  // 임시 경로로 설정
        nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [](HRESULT result, ICoreWebView2Environment* env) -> HRESULT
            {
                if (FAILED(result))
                {
                    MessageBox(nullptr, L"앱을 다시 실행해주세요.", L"오류", MB_OK | MB_ICONERROR);
                    // MessageBox(nullptr, L"WebView2 환경 생성 실패", L"오류", MB_OK | MB_ICONERROR);
                    return result;
                }
                g_webviewEnvironment = env;

                g_webviewEnvironment->CreateCoreWebView2Controller(
                    g_parentWindow,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
                        {
                            if (FAILED(result))
                            {
                                MessageBox(nullptr, L"앱을 다시 실행해주세요.", L"오류", MB_OK | MB_ICONERROR);
                                // MessageBox(nullptr, L"WebView2 컨트롤러 생성 실패", L"오류", MB_OK | MB_ICONERROR);
                                return result;
                            }

                            g_webviewController = controller;
                            g_webviewController->get_CoreWebView2(&g_webviewWindow);

                            ResizeWebView();

                            g_webviewController->put_IsVisible(TRUE);
                            g_webviewWindow->Navigate(WEB_URL);

                            return S_OK;
                        }).Get());

                return S_OK;
            }).Get());

    if (FAILED(hr))
    {
        MessageBox(parentWindow, L"앱을 다시 실행해주세요.", L"오류", MB_OK | MB_ICONERROR);
    }
}
