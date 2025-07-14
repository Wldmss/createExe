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

HWND g_headerWindow = nullptr;
HWND g_exitButton = nullptr;

LRESULT CALLBACK HeaderWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_COMMAND: {
            int controlID = LOWORD(wParam);
            if (controlID == ID_EXIT_BUTTON) {
                // 평가 종료 버튼 클릭
                int result = MessageBox(g_mainWindow, L"시험을 종료하시겠습니까?", L"확인", MB_OKCANCEL | MB_ICONQUESTION);
                if (result == IDOK) {
                    PostMessage(g_mainWindow, WM_DESTROY, 0, 0);
                }
            }
            break;
        }
        case WM_ERASEBKGND:
            // 헤더 배경색 삭제
            return (LRESULT)GetStockObject(WHITE_BRUSH);
        case WM_PAINT: {
            // 헤더 배경색
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(hdc, &clientRect, hBrush);
            DeleteObject(hBrush);

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_SIZE: {
            // 헤더 윈도우 크기 변경 시 버튼 위치 조정
            if (g_exitButton) {
                RECT rcHeader;
                GetClientRect(hwnd, &rcHeader);
                MoveWindow(g_exitButton, rcHeader.right - 110, 10, 100, 30, TRUE);
            }
            break;
        }
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// 헤더와 버튼 생성 (헤더 배경 흰색, 버튼 오른쪽)
void CreateHeader(HWND parent, HINSTANCE hInst) {
    RECT rc;
    GetClientRect(parent, &rc);

    g_headerWindow = CreateWindowEx(
        WS_EX_NOACTIVATE, HEADER_CLASS_NAME, L"",
        WS_CHILD | WS_VISIBLE | SS_NOTIFY,
        0, 0, rc.right, HEADER_HEIGHT,
        parent, (HMENU)ID_HEADER, hInst, nullptr);

    g_exitButton = CreateWindow(
        L"BUTTON", L"평가 종료",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        rc.right - 110, 10, 100, 30,
        g_headerWindow, (HMENU)ID_EXIT_BUTTON, hInst, nullptr);
}

// 평가 종료 버튼
// HWND CreateExitButton(HWND hwnd, HINSTANCE hInst) {
//     return CreateWindow(
//         L"BUTTON", L"평가 종료",
//         WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
//         0, 0, 100, 70,
//         hwnd,
//         (HMENU)ID_EXIT_BUTTON,
//         hInst,
//         nullptr
//     );
// }

// 윈도우 크기 변경 시 WebView 크기 맞추기
void ResizeWebView()
{
    if (g_webviewController && g_parentWindow)
    {
        RECT bounds;
        GetClientRect(g_parentWindow, &bounds);
        bounds.top += HEADER_HEIGHT; // 헤더 높이만큼 아래로 이동
        g_webviewController->put_Bounds(bounds);

        // 헤더와 버튼도 리사이즈
        if (g_headerWindow) {
            MoveWindow(g_headerWindow, 0, 0, bounds.right, HEADER_HEIGHT, TRUE);
        }
        if (g_exitButton) {
            MoveWindow(g_exitButton, bounds.right - 110, 10, 100, 30, TRUE);
        }
    }
}

// WebView javascript 핸들러 설정
void InjectedJavascript() {
    // 버튼 클릭 이벤트
    g_webviewWindow->AddScriptToExecuteOnDocumentCreated(
        L"document.addEventListener('DOMContentLoaded', function() {"
        L"  var btn = document.getElementById('loginMsg');"
        L"  if(btn) {"
        L"    btn.addEventListener('click', function() {"
        L"      window.chrome.webview.postMessage('buttonClicked');"
        L"    });"
        L"  }"
        L"});",
        nullptr);

    g_webviewWindow->add_WebMessageReceived(
        Callback<ICoreWebView2WebMessageReceivedEventHandler>(
            [](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                wil::unique_cotaskmem_string message;
                args->get_WebMessageAsJson(&message);
                std::wstring msg = message.get();
                if (msg == L"\"buttonClicked\"") {
                    MessageBox(nullptr, L"웹 버튼이 클릭되었습니다!", L"알림", MB_OK);
                }
                return S_OK;
            }).Get(), nullptr);

    // 특정 URL 접속 이벤트
    g_webviewWindow->add_NavigationCompleted(
        Callback<ICoreWebView2NavigationCompletedEventHandler>(
            [](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
                wil::unique_cotaskmem_string uri;
                sender->get_Source(&uri);
                std::wstring targetUrl = L"myCourseList.do";
                if (uri && std::wstring(uri.get()).find(targetUrl) != std::wstring::npos) {
                    MessageBox(nullptr, L"특정 URL에 접속했습니다!", L"알림", MB_OK);
                }
                return S_OK;
            }).Get(), nullptr);
}

// webview 생성 이후 처리
void SetAfterWebviewCreated(HWND parent, HINSTANCE hInst) {
    CreateHeader(parent, hInst);
    ResizeWebView();
    InjectedJavascript();
}

// webview load
void LaunchWebView(HWND parent, HINSTANCE hInst)
{
    g_parentWindow = parent;

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
        MessageBox(parent, L"임시 폴더를 가져올 수 없습니다.", L"오류", MB_OK | MB_ICONERROR);
        return;
    }

    HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
        nullptr,
        userDataFolder.c_str(),  // 임시 경로로 설정
        nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [parent, hInst](HRESULT result, ICoreWebView2Environment* env) -> HRESULT
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
                        [parent, hInst](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
                        {
                            if (FAILED(result))
                            {
                                MessageBox(nullptr, L"앱을 다시 실행해주세요.", L"오류", MB_OK | MB_ICONERROR);
                                // MessageBox(nullptr, L"WebView2 컨트롤러 생성 실패", L"오류", MB_OK | MB_ICONERROR);
                                return result;
                            }

                            g_webviewController = controller;
                            g_webviewController->get_CoreWebView2(&g_webviewWindow);
                            g_webviewController->put_IsVisible(TRUE);
                            g_webviewWindow->Navigate(WEB_URL);

                            SetAfterWebviewCreated(parent, hInst);

                            return S_OK;
                        }).Get());

                return S_OK;
            }).Get());

    if (FAILED(hr))
    {
        MessageBox(parent, L"앱을 다시 실행해주세요.", L"오류", MB_OK | MB_ICONERROR);
    }
}