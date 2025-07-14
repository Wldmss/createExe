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