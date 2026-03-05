@echo off
echo ========================================
echo Compiling Isometric Positioning Tests
echo ========================================

cd /d %~dp0

echo.
echo Compiling exploratory tests...

REM Спроба компіляції з різними можливими шляхами до компілятора
if exist "C:\raylib\w64devkit\bin\g++.exe" (
    echo Using Raylib w64devkit...
    "C:\raylib\w64devkit\bin\g++.exe" -std=c++17 -Wall -I../src -I"C:\raylib\raylib\include" -L"C:\raylib\raylib\lib" -o test_isometric_positioning_bugs.exe test_isometric_positioning_bugs.cpp -lraylib -lopengl32 -lgdi32 -lwinmm
) else if exist "C:\raylib\mingw\bin\g++.exe" (
    echo Using Raylib MinGW...
    "C:\raylib\mingw\bin\g++.exe" -std=c++17 -Wall -I../src -I"C:\raylib\include" -L"C:\raylib\lib" -o test_isometric_positioning_bugs.exe test_isometric_positioning_bugs.cpp -lraylib -lopengl32 -lgdi32 -lwinmm
) else if exist "C:\MinGW\bin\g++.exe" (
    echo Using system MinGW...
    "C:\MinGW\bin\g++.exe" -std=c++17 -Wall -I../src -o test_isometric_positioning_bugs.exe test_isometric_positioning_bugs.cpp -lraylib -lopengl32 -lgdi32 -lwinmm
) else (
    echo Trying system g++...
    g++ -std=c++17 -Wall -I../src -o test_isometric_positioning_bugs.exe test_isometric_positioning_bugs.cpp -lraylib -lopengl32 -lgdi32 -lwinmm
)

if not exist "test_isometric_positioning_bugs.exe" (
    echo ERROR: Compilation failed!
    exit /b 1
)

echo.
echo ========================================
echo Running Exploratory Tests
echo ========================================
echo.

test_isometric_positioning_bugs.exe

echo.
echo ========================================
echo Tests Completed
echo ========================================
