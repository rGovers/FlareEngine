#!/bin/bash

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DGENERATE_CONFIG=ON ../cpp/
make -j6
msbuild ../cs/FlareCS.csproj -p:Configuration=Debug