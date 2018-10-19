if not exist "build" mkdir build
REM python compile_shader.py
cd .\build
cmake .. -G "Visual Studio 15 2017 Win64"
REM cmake --build . --target TestD3D --config Debug
REM cmake --build . --target Editor --config Debug
cmake --build . --target ModelViewer --config Debug
pause