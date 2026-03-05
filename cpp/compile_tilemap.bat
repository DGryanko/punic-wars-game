@echo off
echo Compiling Isometric Tilemap Generator...
echo.

REM Визначення компілятора та додавання до PATH
if exist "C:\raylib\w64devkit\bin\g++.exe" (
    echo Using Raylib w64devkit...
    set PATH=C:\raylib\w64devkit\bin;%PATH%
    set GCC=g++
    set RAYLIB_INCLUDE=-IC:\raylib\raylib\include
    set RAYLIB_LIB=-LC:\raylib\raylib\lib
) else if exist "C:\raylib\mingw\bin\g++.exe" (
    echo Using Raylib MinGW...
    set PATH=C:\raylib\mingw\bin;%PATH%
    set GCC=g++
    set RAYLIB_INCLUDE=-IC:\raylib\include
    set RAYLIB_LIB=-LC:\raylib\lib
) else (
    echo Using system g++...
    set GCC=g++
    set RAYLIB_INCLUDE=
    set RAYLIB_LIB=
)

REM Компіляція всіх файлів
%GCC% -std=c++17 -O2 -Isrc %RAYLIB_INCLUDE% -c src/tilemap/tilemap.cpp -o src/tilemap/tilemap.o
%GCC% -std=c++17 -O2 -Isrc %RAYLIB_INCLUDE% -c src/tilemap/noise.cpp -o src/tilemap/noise.o
%GCC% -std=c++17 -O2 -Isrc %RAYLIB_INCLUDE% -c src/tilemap/map_generator.cpp -o src/tilemap/map_generator.o
%GCC% -std=c++17 -O2 -Isrc %RAYLIB_INCLUDE% -c src/tilemap/isometric_renderer.cpp -o src/tilemap/isometric_renderer.o
%GCC% -std=c++17 -O2 -Isrc %RAYLIB_INCLUDE% -c src/tilemap/map_serializer.cpp -o src/tilemap/map_serializer.o

REM Лінкування
%GCC% -std=c++17 -O2 -Isrc %RAYLIB_INCLUDE% -o tilemap_test.exe src/tilemap_test.cpp src/tilemap/tilemap.o src/tilemap/noise.o src/tilemap/map_generator.o src/tilemap/isometric_renderer.o src/tilemap/map_serializer.o %RAYLIB_LIB% -lraylib -lopengl32 -lgdi32 -lwinmm

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Compilation successful!
    echo Run: tilemap_test.exe
) else (
    echo.
    echo Compilation failed!
)
