#!/bin/bash

mkdir crossBuild
cd crossBuild
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-O3 -DGENERATE_CONFIG=ON -DCMAKE_TOOLCHAIN_FILE=../toolchains/WinCross.cmake ../FlareNative/
make -j6

../deps/flare-mono/build/bin/xbuild ../FlareCS/FlareCS.csproj /p:Configuration=Release

cd ../bin

cp -r ../deps/flare-mono/crossbuild/lib .
cp -r ../deps/flare-mono/crossbuild/etc .

mv FlareCS.exe FlareCS.dll