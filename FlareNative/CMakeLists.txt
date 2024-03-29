cmake_minimum_required (VERSION 3.8)

project (FlareNative VERSION 0.1 LANGUAGES C CXX)

option(GENERATE_CONFIG "Generates engine configuration file" OFF)
option(ENABLE_TRACE "Enables tracing" OFF)
option(ENABLE_PROFILER "Enables inbuilt profiler" OFF)
option(ENABLE_ASSERT "Enables flare asserts" OFF)
option(OUTPUT_BIN "Outputs application to bin/" OFF)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT WIN32)
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
    set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address")
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules/")

include("FileToHeader")

find_package(Vulkan REQUIRED)

set (FLARENATIVE_VERSION_MAJOR ${CMAKE_PROJECT_VERSION_MAJOR})
set (FLARENATIVE_VERSION_MINOR ${CMAKE_PROJECT_VERSION_MINOR})

configure_file(
    "${PROJECT_SOURCE_DIR}/FlareNativeConfig.h.in"
    "${PROJECT_SOURCE_DIR}/include/FlareNativeConfig.h"
)

if (ENABLE_TRACE)
    add_compile_definitions(FLARENATIVE_ENABLE_TRACE)
endif()
if (ENABLE_PROFILER)
    add_compile_definitions(FLARENATIVE_ENABLE_PROFILER)
endif()
if (ENABLE_ASSERT)
    add_compile_definitions(FLARE_ENABLE_ASSERT)
endif()

add_compile_definitions(GLM_FORCE_QUAT_DATA_XYZW)
add_compile_definitions(GLM_FORCE_DEPTH_ZERO_TO_ONE)
add_compile_definitions(GLM_FORCE_RADIANS)

if (GENERATE_CONFIG)
    configure_file(
        "${PROJECT_SOURCE_DIR}/config.xml.in"
        "${PROJECT_SOURCE_DIR}/../bin/config.xml"
    )
endif()

FileToHeader(
    SOURCE_FILE "shaders/quad.vert"
    HEADER_FILE "include/Shaders/QuadVertex.h"
    VARIABLE_NAME "QuadVertex"
)

FileToHeader(
    SOURCE_FILE "shaders/directionalLight.fpix"
    HEADER_FILE "include/Shaders/DirectionalLightPixel.h"
    VARIABLE_NAME "DirectionalLightPixel"
)
FileToHeader(
    SOURCE_FILE "shaders/pointLight.fpix"
    HEADER_FILE "include/Shaders/PointLightPixel.h"
    VARIABLE_NAME "PointLightPixel"
)
FileToHeader(
    SOURCE_FILE "shaders/spotLight.fpix"
    HEADER_FILE "include/Shaders/SpotLightPixel.h"
    VARIABLE_NAME "SpotLightPixel"
)

FileToHeader(
    SOURCE_FILE "shaders/post.fpix"
    HEADER_FILE "include/Shaders/PostPixel.h"
    VARIABLE_NAME "PostPixel"
)

if (MINGW)
    include_directories("${PROJECT_SOURCE_DIR}/../deps/flare-mono/crossbuild/include/mono-2.0/")
    link_directories("${PROJECT_SOURCE_DIR}/../deps/flare-mono/crossbuild/lib/")
else()
    include_directories("${PROJECT_SOURCE_DIR}/../deps/flare-mono/build/include/mono-2.0/")
    link_directories("${PROJECT_SOURCE_DIR}/../deps/flare-mono/build/lib/")
endif()

include_directories("${PROJECT_SOURCE_DIR}/include/"
                    "${PROJECT_SOURCE_DIR}/../FlareBase/include/"
                    "${PROJECT_SOURCE_DIR}/../deps/flare-glfw/include/"
                    "${PROJECT_SOURCE_DIR}/../deps/flare-glm/"
                    "${PROJECT_SOURCE_DIR}/../deps/flare-stb/"
                    "${PROJECT_SOURCE_DIR}/../deps/flare-tinyxml2/"
                    "${PROJECT_SOURCE_DIR}/lib/glslang/"
                    "${PROJECT_SOURCE_DIR}/lib/VulkanMemoryAllocator/include/"
                    ${Vulkan_INCLUDE_DIRS})

file(GLOB SOURCES 
    "src/*.cpp" 
    "../deps/flare-tinyxml2/tinyxml2.cpp")

set(SKIP_GLSLANG_INSTALL ON)
set(ENABLE_GLSLANG_BINARIES OFF)
set(BUILD_EXTERNAL OFF)
set(ENABLE_CTEST OFF)

if (NOT TARGET glfw)
    add_subdirectory("${PROJECT_SOURCE_DIR}/../deps/flare-glfw/" "build/glfw")
endif()
if (NOT TARGET FlareBase)
    add_subdirectory("${PROJECT_SOURCE_DIR}/../FlareBase" "build/FlareBase")
endif()
add_subdirectory("${PROJECT_SOURCE_DIR}/lib/glslang")

add_executable(FlareNative ${SOURCES})
target_link_libraries(FlareNative FlareBase glfw glslang SPIRV ${GLFW_LIBRARIES} ${Vulkan_LIBRARIES})

if (OUTPUT_BIN)
    set_target_properties(FlareNative
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/"
    )
endif()

if (WIN32)
    target_link_libraries(FlareNative wsock32 ws2_32)

    if (MSVC)    
        target_link_libraries(FlareNative mono-2.0-sgen)
        set_property(TARGET FlareNative PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
    else()
        target_link_libraries(FlareNative monosgen-2.0)

        target_link_libraries(FlareNative -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic)
    endif()
else()
    target_link_libraries(FlareNative mono-2.0)
endif()