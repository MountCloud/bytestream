rm -rf build
mkdir build
rm -rf bin
mkdir bin

cd build

cmake -DCMAKE_BUILD_TYPE=Debug .. && cmake --build . --config Debug

cd ..