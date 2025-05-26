* 릴리즈 빌드
cmake -B build/Release -G "Visual Studio 17 2022"
cmake --build build/Release --config Release

<!-- cmake -DCMAKE_BUILD_TYPE=Release -B build/release
cmake --build build/release -->

* Debug 빌드
Ctrl+Shift+P > Cmake:Configuration
Ctrl+Shift+P > Cmake:Build