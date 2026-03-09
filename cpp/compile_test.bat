@echo off
echo Compiling refactoring test...
C:\raylib\w64devkit\bin\g++.exe tests/test_refactoring.cpp -o tests/test_refactoring.exe -std=c++17
if %errorlevel% equ 0 (
    echo Compilation successful!
    echo Running test...
    tests\test_refactoring.exe
) else (
    echo Compilation failed!
)
