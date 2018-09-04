cd ./ThirdParty
// cd bgfx && git pull && cd ..
// cd bx && git pull && cd ..
// cd bimg && git pull && cd ..
cd bgfx
../bx/tools/bin/darwin/genie --with-glfw --with-tools --gcc=osx --platform=x64 gmake
# ../bx/tools/bin/darwin/genie --with-glfw --with-tools --gcc=osx --platform=universal64 xcode9
cd .build/projects/gmake-osx
make config=debug64
make config=release64
