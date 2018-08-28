cd ../github
cd bgfx && git pull && cd ..
cd bx && git pull && cd ..
cd bimg && git pull && cd ..
cd bgfx
../bx/tools/bin/darwin/genie --with-glfw --gcc=osx --platform=x64 gmake
cd .build/projects/gmake-osx
make config=release64