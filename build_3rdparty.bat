cd ./ThirdParty
REM cd bgfx && git pull && cd ..
REM cd bx && git pull && cd ..
REM cd bimg && git pull && cd ..
cd tinygltf && git pull && cd ..
cd glfw && git pull && cd ..
cd imgui && git pull && cd ..
REM cd bgfx
REM ..\bx\tools\bin\windows\genie --with-glfw --platform=x64 --with-tools --with-dynamic-runtime --with-windows=10.0.17134.0 --with-examples vs2017
REM "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\IDE\devenv" .\.build\projects\vs2017\bgfx.sln /Build "Debug|x64"
REM "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\IDE\devenv" .\.build\projects\vs2017\bgfx.sln /Build "Release|x64"
pause