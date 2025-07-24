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

#include <urlmon.h>
#include <shellapi.h>
#include <ShlObj.h>

#include <regex>
#include <shlwapi.h>
#include <set>

using namespace Microsoft::WRL;

// 전역 변수로 WebView2 객체들 유지 (view.cpp 내 전역)
static wil::com_ptr<ICoreWebView2Environment> g_webviewEnvironment;
static wil::com_ptr<ICoreWebView2Controller> g_webviewController;
static wil::com_ptr<ICoreWebView2> g_webviewWindow;

static HWND g_parentWindow = nullptr;
const wchar_t WEB_URL[] = L"https://ktedu.kt.com";

HWND g_headerWindow = nullptr;
HWND g_exitButton = nullptr;
HWND g_execPPTButton = nullptr;

// EnumWindows 콜백에 데이터를 전달하기 위한 구조체
struct EnumDataForClassName {
    const wchar_t* className;
    std::set<HWND> windowHandles;
};

// 특정 클래스 이름을 가진 모든 창을 찾아 std::set에 추가하는 콜백 함수
BOOL CALLBACK EnumWindowsProcByClassName(HWND hWnd, LPARAM lParam) {
    auto* pData = (EnumDataForClassName*)lParam;
    wchar_t currentClassName[256];
    GetClassNameW(hWnd, currentClassName, 256);

    if (wcscmp(currentClassName, pData->className) == 0) {
        pData->windowHandles.insert(hWnd);
    }
    return TRUE; // 모든 창을 계속 검색
}

// 스레드에 넘겨줄 데이터 구조체
struct MonitorThreadParams {
    HWND hMainWnd;
    HANDLE hProcess;
    HWND hPptWnd;
};

// 파워포인트 프로세스의 종료 및 포커스를 감시하는 스레드 함수
DWORD WINAPI MonitorPowerPointThread(LPVOID lpParam)
{
    auto* params = (MonitorThreadParams*)lpParam;

    if (params && params->hPptWnd) 
    {
        printf("Start monitoring PowerPoint window: %p\n", params->hPptWnd);
        PostMessage(params->hMainWnd, WM_HIDETASKBAR, 0, 0);
        
        // 메인 감시 루프: 전달받은 '창'이 유효한 동안 반복
        while (IsWindow(params->hPptWnd)) 
        {
            HWND hForegroundWnd = GetForegroundWindow();
            printf("Current foreground window: %p\n", hForegroundWnd);

            // 현재 활성창이 PPT창도 아니고 우리 메인 앱 창도 아니라면 포커스 되돌림
            if (hForegroundWnd != params->hPptWnd && hForegroundWnd != params->hMainWnd) {
                printf("Restoring focus to PowerPoint window\n");
                wchar_t className[256];
                GetClassNameW(hForegroundWnd, className, 256);
                if (wcscmp(className, L"#32770") != 0) {
                    MessageBox(params->hMainWnd, L"평가 중 다른 프로그램 실행 시 평가에 불이익이 있을 수 있습니다.", L"경고", MB_OK | MB_ICONWARNING | MB_TOPMOST);

                    SetForegroundWindow(params->hMainWnd);
                    if (IsWindow(params->hPptWnd)) {
                        SetForegroundWindow(params->hPptWnd);   // ppt로 focus
                    }
                }
            }
            Sleep(250);
        }
        printf("PowerPoint window was closed.\n");
    } else {
        // 창 핸들을 못 찾은 비상시에는 기존처럼 프로세스 종료만 대기
        printf("Could not find PowerPoint window. Waiting for initial process to terminate.\n");
        WaitForSingleObject(params->hProcess, INFINITE);
    }

    // 정리 작업
    if (params) {
        if (params->hProcess) CloseHandle(params->hProcess);
        if (params->hMainWnd) {
            printf("PowerPoint process end...\n");
            PostMessage(params->hMainWnd, WM_SHOWTASKBAR, 0, 0);
            PostMessage(params->hMainWnd, WM_RESTORE_FOCUS, 0, 0);
        }
        delete params;
    }
    return 0;
}

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
            } else if (controlID == ID_EXEC_PPT) {
                std::wstring fileUrl;

                int msgResult = MessageBox(g_mainWindow, L"PPT 파일을 다운로드하여 실행하시겠습니까?", L"확인", MB_OKCANCEL | MB_ICONQUESTION);
                if (msgResult == IDOK) {

                    // 다운로드할 파일 URL
                    // fileUrl = L"C:\\Users\\sqi\\Downloads\\대량평가_답변_김지은.pptx";
                    fileUrl = L"https://github.com/Wldmss/drive/raw/refs/heads/main/%EB%8C%80%EB%9F%89%ED%8F%89%EA%B0%80_%EB%8B%B5%EB%B3%80_%EA%B9%80%EC%A7%80%EC%9D%80.pptx";

                    if (fileUrl.empty()) {
                        MessageBox(g_mainWindow, L"다운로드 URL이 지정되지 않았습니다.", L"오류", MB_OK | MB_ICONERROR);
                        return 0;
                    }

                    // 다운로드 폴더 경로 가져오기
                    std::wstring downloadFolder;
                    PWSTR pszPath = NULL;
                    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Downloads, 0, NULL, &pszPath);
                    if (SUCCEEDED(hr)) {
                        downloadFolder = pszPath;
                        if (!downloadFolder.empty() && downloadFolder.back() != L'\\')
                        {
                            downloadFolder += L'\\';
                        }
                    } else {
                        MessageBox(g_mainWindow, L"다운로드 폴더 경로를 찾을 수 없습니다.", L"오류", MB_OK | MB_ICONERROR);
                        return 0;
                    }

                    // 파일명과 확장자 설정
                    const wchar_t* baseFileName = L"대량평가_답변_김지은";
                    const wchar_t* extension = L".pptx";

                    wchar_t finalFilePath[MAX_PATH];
                    swprintf_s(finalFilePath, MAX_PATH, L"%s%s%s", downloadFolder.c_str(), baseFileName, extension);

                    const bool passDownload = false; // TEST 용, 실제 다운로드를 건너뛰고 바로 실행
                    bool doOpen = true;
                    
                    if(!passDownload) {
                        // 파일명 중복 확인
                        int counter = 1;
                        while (PathFileExistsW(finalFilePath))
                        {
                            swprintf_s(finalFilePath, MAX_PATH, L"%s%s(%d)%s", downloadFolder.c_str(), baseFileName, counter, extension);
                            counter++;
                        }

                        // 파일 다운로드
                        HRESULT hr2 = URLDownloadToFileW(NULL, fileUrl.c_str(), finalFilePath, 0, NULL);
                        doOpen = SUCCEEDED(hr2);
                    }
                    
                    printf("Download...: %ls\n", finalFilePath);

                    // 다운로드 성공 여부 확인
                    if (doOpen && PathFileExistsW(finalFilePath))
                    {
                        // PPT 실행 전, 이미 열려있는 PPT 창 목록을 저장
                        EnumDataForClassName beforeData = { L"PPTFrameClass" };
                        EnumWindows(EnumWindowsProcByClassName, (LPARAM)&beforeData);

                        wchar_t params[MAX_PATH + 3];
                        swprintf_s(params, MAX_PATH + 3, L"\"%s\"", finalFilePath); // 편집 모드

                        SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
                        sei.fMask = SEE_MASK_NOCLOSEPROCESS; // 프로세스 핸들을 받기 위한 플래그
                        sei.hwnd = NULL;
                        sei.lpVerb = L"open";
                        sei.lpFile = L"POWERPNT.EXE"; // 파워포인트 실행 파일
                        sei.lpParameters = params;     // 파라미터: /S "파일경로"
                        sei.nShow = SW_MAXIMIZE;

                        // 3. 파워포인트 실행
                        if (ShellExecuteEx(&sei) && sei.hProcess != NULL)
                        {
                            // 새 창이 완전히 열릴 때까지 잠시 대기 (네트워크나 PC 환경에 따라 조절)
                            Sleep(2000);

                            // PPT 실행 후, 다시 창 목록을 가져와 '새로 생긴 창'을 찾음
                            EnumDataForClassName afterData = { L"PPTFrameClass" };
                            EnumWindows(EnumWindowsProcByClassName, (LPARAM)&afterData);
                            
                            HWND hNewPptWnd = NULL;
                            for (HWND hWnd : afterData.windowHandles) {
                                // 실행 후 목록에 있지만, 실행 전 목록에는 없던 창이 바로 새로 생긴 창
                                if (beforeData.windowHandles.find(hWnd) == beforeData.windowHandles.end()) {
                                    hNewPptWnd = hWnd;
                                    break;
                                }
                            }

                            // 새로 찾은 창 핸들을 감시 스레드에 넘겨줌
                            if (hNewPptWnd) {
                                MonitorThreadParams* threadParams = new MonitorThreadParams();
                                threadParams->hMainWnd = g_mainWindow;
                                threadParams->hProcess = sei.hProcess; // 예비용으로 계속 전달
                                threadParams->hPptWnd = hNewPptWnd;    // 새로 찾은 창 핸들 전달

                                CreateThread(NULL, 0, MonitorPowerPointThread, threadParams, 0, NULL);
                            } else {
                                MessageBox(g_mainWindow, L"새로운 PowerPoint 창을 감지할 수 없습니다.", L"오류", MB_OK);
                                CloseHandle(sei.hProcess);
                            }
                        }
                        else
                        {
                            MessageBox(g_mainWindow, L"PowerPoint를 실행할 수 없습니다.", L"실행 오류", MB_OK | MB_ICONERROR);
                        }
                    }
                    else
                    {
                        MessageBox(g_mainWindow, L"파일을 다운로드하는 데 실패했습니다.\n다시 시도해주세요.", L"다운로드 오류", MB_OK | MB_ICONERROR);
                    }
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

            if (g_execPPTButton) {
                RECT rcHeader;
                GetClientRect(hwnd, &rcHeader);
                MoveWindow(g_execPPTButton, rcHeader.right - 310, 10, 150, 30, TRUE);
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

    g_execPPTButton = CreateWindow(
        L"BUTTON", L"파워포인트 실행", 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        rc.right - 310, 10, 150, 30,
        g_headerWindow, (HMENU)ID_EXEC_PPT, hInst, nullptr);
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
        if (g_execPPTButton) {
            MoveWindow(g_execPPTButton, bounds.right - 310, 10, 150, 30, TRUE);
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
                    // MessageBox(nullptr, L"웹 버튼이 클릭되었습니다!", L"알림", MB_OK);
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
                    // MessageBox(nullptr, L"특정 URL에 접속했습니다!", L"알림", MB_OK);
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