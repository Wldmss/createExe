cmake_minimum_required(VERSION 3.10)
project(ktGeniusExam)

set(CMAKE_CXX_STANDARD 17)
set(TARGET_NAME ktGeniusExam)
set(EXE_NAME "지니어스 대량평가")
set(EXE_VERSION 0.0.1-251001)

add_definitions(-DUNICODE -D_UNICODE)

# 리소스 파일 경로
set(RESOURCE_FILES
    resources/ktGeniusIcon.rc
)

# 빌드 대상
# add_executable(${TARGET_NAME}
#     src/main.cpp
#     src/view.cpp
#     src/monitor.cpp
#     src/keypress.cpp
#     ${RESOURCE_FILES}
# )
add_executable(${TARGET_NAME} WIN32
    src/main.cpp
    src/view.cpp
    src/monitor.cpp
    src/keypress.cpp
    ${RESOURCE_FILES}
)

# 빌드 타입별 출력 디렉토리 설정 (Debug/Release 구분)
set_target_properties(${TARGET_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/build/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/build/Release"
    OUTPUT_NAME "${EXE_NAME}"
)
# 출력 디렉토리 설정
# set_target_properties(${TARGET_NAME} PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build"
# )

# include 디렉토리 추가
target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib
)

# 라이브러리 링크
target_link_libraries(${TARGET_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/WebView2LoaderStatic.lib
    user32.lib
    ole32.lib
    oleaut32.lib
    gdi32.lib
    windowscodecs.lib
    shell32.lib
    urlmon.lib
    shlwapi.lib
)

# -------------------------------
# 📦 CPack을 이용한 인스톨러 설정
# -------------------------------
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "${EXE_NAME}")
set(CPACK_PACKAGE_VENDOR "KT Corporation")     # 회사명/조직명
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${TARGET_NAME} Application")
set(CPACK_PACKAGE_VERSION "${EXE_VERSION}")     # 버전
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${TARGET_NAME}")
set(CPACK_PACKAGE_EXECUTABLES ${TARGET_NAME} "${TARGET_NAME}")

# 설치 경로에 실행파일 복사
install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)

# 설치 옵션 (NSIS 또는 ZIP 등)
set(CPACK_GENERATOR "NSIS")   # Windows 설치 exe
# set(CPACK_GENERATOR "ZIP")  # 압축 파일 배포

set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_NAME}")
set(CPACK_NSIS_PACKAGE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")

# 설치 프로그램 아이콘
set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/resources/ktgenius-icon.ico")

# 설치 시 바로가기 설정
set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
  "CreateShortCut \\\"$PROFILE\\\\Desktop\\\\${EXE_NAME}.lnk\\\" \\\"$INSTDIR\\\\bin\\\\${EXE_NAME}.exe\\\" \\n
   CreateShortCut \\\"$SMPROGRAMS\\\\${EXE_NAME}.lnk\\\" \\\"$INSTDIR\\\\bin\\\\${EXE_NAME}.exe\\\""
)

# 삭제 시 바로가기 해제
set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS
  "Delete \\\"$PROFILE\\\\DESKTOP\\\\${EXE_NAME}.lnk\\\" \\n
   Delete \\\"$SMPROGRAMS\\\\${EXE_NAME}.lnk\\\""
)

# set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
#   ; 설치 완료 후 실행 파일 위치 열기
#   ExecShell open \\\"$INSTDIR\\\\bin\\\"
# ")

include(CPack)