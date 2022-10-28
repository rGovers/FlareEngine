#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <stdio.h>
#include <string_view>

#include "FlareNativeConfig.h"
#include "Application.h"

constexpr std::string_view ExecDir = "./Flare.dll";

int main(int a_argc, char* a_argv[])
{
    printf("FlareEngine %d.%d \n", FLARENATIVE_VERSION_MAJOR, FLARENATIVE_VERSION_MINOR);

    Application app;
    app.Run((int32_t)a_argc, a_argv);

    return 0;
}