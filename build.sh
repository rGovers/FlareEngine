#!/bin/bash

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DGENERATE_CONFIG=ON ../cpp/
make -j6
msbuild ../cs/FlareCS.csproj -p:Configuration=Release
mv ../bin/FlareCS.exe ../bin/FlareCS.dll