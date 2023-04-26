#!/bin/bash

if [ ! -d "build" ]; then
    mkdir build
fi
cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DGENERATE_CONFIG=ON -DENABLE_TRACE=ON -DENABLE_PROFILER=ON ../FlareNative/
make -j6
../deps/flare-mono/build/bin/xbuild ../FlareCS/FlareCS.csproj /p:Configuration=Release

cd ../bin/
cp -r ../deps/flare-mono/build/lib .
cp -r ../deps/flare-mono/build/etc .
mv FlareCS.exe FlareCS.dllcl