#!/bin/bash

if [ ! -d "build" ]; then
    mkdir build
fi
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-O3 -DGENERATE_CONFIG=ON ../FlareNative/
make -j6
../deps/flare-mono/build/bin/xbuild ../FlareCS/FlareCS.csproj /p:Configuration=Release

cd ../bin/
cp -r ../deps/flare-mono/build/lib .
cp -r ../deps/flare-mono/build/etc .
mv FlareCS.exe FlareCS.dll