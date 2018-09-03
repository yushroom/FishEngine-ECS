cd ./ThirdParty
rem cd bgfx && git pull && cd ..
rem cd bx && git pull && cd ..
rem cd bimg && git pull && cd ..
cd bgfx
..\bx\tools\bin\windows\genie --with-glfw --platform=x64 --with-tools --with-dynamic-runtime --with-windows=10.0.17134.0 --with-examples vs2017
pause