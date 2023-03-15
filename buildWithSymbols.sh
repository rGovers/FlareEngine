#!/bin/bash

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DGENERATE_CONFIG=ON -DENABLE_TRACE=ON -DENABLE_PROFILER=ON ../cpp/
make -j6
../deps/flare-mono/build/bin/xbuild ../cs/FlareCS.csproj /p:Configuration=Release

cd ../bin/
cp -r ../deps/flare-mono/build/lib .
cp -r ../deps/flare-mono/build/etc .
mv FlareCS.exe FlareCS.dll