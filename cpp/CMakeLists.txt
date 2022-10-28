cmake_minimum_required (VERSION 3.0)

project (FlashFireNative VERSION 0.1 LANGUAGES C CXX)

option(GENERATE_CONFIG "Generates engine configuration file" OFF)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT WIN32)
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
    set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address")
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules/")

find_package(PkgConfig REQUIRED)

pkg_check_modules(MONO REQUIRED mono-2)

set (FLASHFIRENATIVE_VERSION_MAJOR ${CMAKE_PROJECT_VERSION_MAJOR})
set (FLASHFIRENATIVE_VERSION_MINOR ${CMAKE_PROJECT_VERSION_MINOR})

configure_file(
    "${PROJECT_SOURCE_DIR}/FlashFireNativeConfig.h.in"
    "${PROJECT_SOURCE_DIR}/include/FlashFireNativeConfig.h"
)

if (GENERATE_CONFIG)
    configure_file(
        "${PROJECT_SOURCE_DIR}/config.xml.in"
        "${PROJECT_SOURCE_DIR}/../bin/config.xml"
    )
endif()

include_directories("${PROJECT_SOURCE_DIR}/include/"
                    "${PROJECT_SOURCE_DIR}/lib/tinyxml2/"
                    ${MONO_INCLUDE_DIRS})

file(GLOB SOURCES "src/*.cpp" "lib/tinyxml2/tinyxml2.cpp")

add_executable(FlashFireNative ${SOURCES})
target_link_libraries(FlashFireNative ${MONO_LIBRARIES})
target_compile_options(FlashFireNative PUBLIC ${MONO_CFLAGS_OTHER})