# 프로젝트 세팅

- vsCode 설치
- vs_Buildtools 설치
    - 'C++을 사용한 데스크톱 개발' 설치
- vsCode extension 설치
    - cmake tools
        https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools
    - c++ extension pack
        https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack
- mingw-get-setup 설치
    - mingw-developer-toolkit
    - mingw32-base
    - mingw-32-gcc-g++
    - msys-base
    -> installation > Apply Changes > Apply

    - PATH 환경변수 설정
        - C:\MinGW\bin
        - C:\MinGW\msys\1.0\bin

# 프로젝트 실행

##  릴리즈 빌드
cmake -B build/Release -G "Visual Studio 17 2022"
cmake --build build/Release --config Release

-> 파일 생성 경로: \build\Release\build\Release\

##  Debug 빌드
Ctrl+Shift+P > Cmake:Configuration [Visual Studio Build Tools 2022 Release - amd64]
Ctrl+Shift+P > Cmake:Build

위에꺼로 빌드가 안되면 터미널에서 아래 명령어 실행
cmake -B build/Debug -G "Visual Studio 17 2022" -A x64
cmake --build build/Debug --config Debug

-> 파일 생성 경로: \build\Debug\build\Debug\

# 주의사항
파일이 utf-8 이어야 정상적으로 실행됨.
가끔 신경을 안쓰면 utf-8 로 변경하다가 한글이 모두 깨질수도 있으니 주의 바람

# 기능
- 특수 키 차단 (keypress.cpp)
    - ctrl, alt, f11, f12, windows
- 듀얼 모니터, 다른 창 감지 (monitor.cpp)
    - 듀얼 모니터 감지 시 앱 실행 불가
    - 1초에 한번씩 다른 창 감지
- webview로 화면 노출 (view.cpp)
- webview에서 평가 종료 화면 표기 시 앱 종료 처리 (view.cpp > InjectedJavascript)