#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <stdio.h>
#include <string.h>
#include <string_view>

#include "Config.h"
#include "FlareNativeConfig.h"
#include "Application.h"

constexpr std::string_view ExecDir = "./Flare.dll";

int main(int a_argc, char* a_argv[])
{
    Config* config = new Config("./config.xml");

    printf("FlareEngine %d.%d \n", FLARENATIVE_VERSION_MAJOR, FLARENATIVE_VERSION_MINOR);

    for (int i = 0; i < a_argc; ++i)
    {
        if (strcmp(a_argv[i], "--headless") == 0)
        {
            config->SetHeadless(true);
        }
    }

    Application app = Application(config);
    app.Run((int32_t)a_argc, a_argv);

    return 0;
}