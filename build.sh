mkdir -p build && cd build
cmake .. -G Xcode
cmake --build . --target Editor --config Debug
