mkdir build
cd build

cmake -G "Visual Studio 17" ..

cd ..

copy "deps\mono\build\bin\mono-2.0-sgen.dll" "bin\mono-2.0-sgen.dll"
xcopy "deps\mono\build\lib\*.*" "bin\lib\" /y /s
xcopy "deps\mono\build\etc\*.*" "bin\etc\" /y /s
