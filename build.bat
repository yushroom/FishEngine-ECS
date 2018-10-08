if not exist "build" mkdir build
python compile_shader.py
cd .\build
cmake .. -G "Visual Studio 15 2017 Win64"
cmake --build . --target TestD3D --config Debug
rem cmake --build . --target Editor --config Release
pause