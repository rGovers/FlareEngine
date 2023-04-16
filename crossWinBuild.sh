#!/bin/bash

mkdir crossBuild
cd crossBuild
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-O3 -DGENERATE_CONFIG=ON -DCMAKE_TOOLCHAIN_FILE=../toolchains/WinCross.cmake ../cpp/
make -j6

../deps/flare-mono/build/bin/xbuild ../cs/FlareCS.csproj /p:Configuration=Release

cd ../bin

cp -r ../deps/flare-mono/crossbuild/lib .
cp -r ../deps/flare-mono/crossbuild/etc .

mv FlareCS.exe FlareCS.dll