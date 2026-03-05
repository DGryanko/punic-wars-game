@echo off
echo Compiling TileMap Geometry Synchronization Tests...
g++ -std=c++17 -I../src test_tilemap_geometry_sync.cpp ../src/tilemap/tilemap.cpp raylib_stubs.cpp -o test_tilemap_geometry_sync.exe
if %errorlevel% neq 0 (
    echo Compilation failed!
    exit /b 1
)

echo Running tests...
test_tilemap_geometry_sync.exe
