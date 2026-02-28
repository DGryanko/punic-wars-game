@echo off
echo Compiling Isometric Tilemap Generator...
echo.

REM Компіляція всіх файлів
g++ -std=c++17 -Wall -Wextra -O2 -Isrc -c src/tilemap/tilemap.cpp -o src/tilemap/tilemap.o
g++ -std=c++17 -Wall -Wextra -O2 -Isrc -c src/tilemap/noise.cpp -o src/tilemap/noise.o
g++ -std=c++17 -Wall -Wextra -O2 -Isrc -c src/tilemap/map_generator.cpp -o src/tilemap/map_generator.o
g++ -std=c++17 -Wall -Wextra -O2 -Isrc -c src/tilemap/isometric_renderer.cpp -o src/tilemap/isometric_renderer.o
g++ -std=c++17 -Wall -Wextra -O2 -Isrc -c src/tilemap/map_serializer.cpp -o src/tilemap/map_serializer.o

REM Лінкування
g++ -std=c++17 -Wall -Wextra -O2 -Isrc -o tilemap_test.exe src/tilemap_test.cpp src/tilemap/tilemap.o src/tilemap/noise.o src/tilemap/map_generator.o src/tilemap/isometric_renderer.o src/tilemap/map_serializer.o -lraylib -lopengl32 -lgdi32 -lwinmm

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Compilation successful!
    echo Run: tilemap_test.exe
) else (
    echo.
    echo Compilation failed!
)

pause
