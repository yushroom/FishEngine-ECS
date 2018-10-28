cd ./ThirdParty
cd glfw && git pull && cd ..
#cd bgfx && git pull && cd ..
#cd bx && git pull && cd ..
#cd bimg && git pull && cd ..
cd imgui && git pull && cd ..
cd DirectXShaderCompiler && git pull && cd ..
cd SPIRV-Cross && git pull && cd ..

#cd bgfx
#../bx/tools/bin/darwin/genie --with-glfw --with-tools --gcc=osx --platform=x64 gmake
# # ../bx/tools/bin/darwin/genie --with-glfw --with-tools --gcc=osx --platform=universal64 xcode9
#cd .build/projects/gmake-osx
#make config=debug64
#make config=release64

#cd DirectXShaderCompiler
#mkdir -p build && cd build
#cmake .. -DCMAKE_BUILD_TYPE=Release $(cat ../utils/cmake-predefined-config-params)
#make dxc
#cd ..

cd SPIRV-Cross
make
cd ..
