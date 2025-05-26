cmake_minimum_required(VERSION 3.10)
project(genius)

set(CMAKE_CXX_STANDARD 17)

add_definitions(-DUNICODE -D_UNICODE)

# 빌드 대상
add_executable(genius WIN32
    src/main.cpp
    src/view.cpp
    src/monitor.cpp
    src/keypress.cpp
)

# 빌드 타입별 출력 디렉토리 설정 (Debug/Release 구분)
set_target_properties(genius PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/build/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/build/Release"
)
# 출력 디렉토리 설정
# set_target_properties(genius PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build"
# )

# include 디렉토리 추가
target_include_directories(genius PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib
)

# 라이브러리 링크
target_link_libraries(genius PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/WebView2LoaderStatic.lib
    user32.lib
    ole32.lib
    oleaut32.lib
    gdi32.lib
    windowscodecs.lib
    shell32.lib
)
