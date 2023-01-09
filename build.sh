#!/bin/bash

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DGENERATE_CONFIG=ON ../cpp/
make -j6
../deps/mono/build/bin/xbuild ../cs/FlareCS.csproj -p:Configuration=Release

cd ../bin/
cp -r ../deps/mono/build/lib .
cp -r ../deps/mono/build/etc .
mv FlareCS.exe FlareCS.dll