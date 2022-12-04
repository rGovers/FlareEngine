#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <stdio.h>
#include <string.h>

#include "Config.h"
#include "FlareNativeConfig.h"
#include "Application.h"

int main(int a_argc, char* a_argv[])
{
    Config* config = new Config("./config.xml");

    printf("FlareEngine %d.%d \n", FLARENATIVE_VERSION_MAJOR, FLARENATIVE_VERSION_MINOR);

    for (int i = 0; i < a_argc; ++i)
    {
        const char* arg = a_argv[i];
        if (strcmp(arg, "--headless") == 0)
        {
            config->SetHeadless(true);
        }
    }

    Application app = Application(config);
    app.Run((int32_t)a_argc, a_argv);

    return 0;
}