python compile_shader.py
cd .\build
cmake .. -G "Visual Studio 15 2017" Win64
cmake --build . --target demo1 --config Debug
pause