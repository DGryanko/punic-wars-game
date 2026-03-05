@echo off
echo Compiling faction buildings visibility test...
g++ -std=c++17 -Wall -I../src -o test_faction_buildings_visibility.exe test_faction_buildings_visibility.cpp
if %errorlevel% neq 0 (
    echo Compilation failed!
    exit /b 1
)

echo.
echo Running test...
echo.
test_faction_buildings_visibility.exe
