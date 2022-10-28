cmake_minimum_required (VERSION 3.14)

project (FlashFireCS VERSION 0.1 LANGUAGES CSharp)

set(CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2")
set(CMAKE_CSharp_FLAGS "/platform:x64")

set_property(TARGET Example PROPERTY WIN32_EXECUTABLE FALSE)

add_executable(FlashFireNative ${SOURCES})