if not exist "build" mkdir build
cd .\build
cmake .. -G "Visual Studio 15 2017 Win64"
REM cmake --build . --target FishEngine --config Debug
pause