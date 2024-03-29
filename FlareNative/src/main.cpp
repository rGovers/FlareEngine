#include <stdio.h>
#include <string.h>

#include "Application.h"
#include "Config.h"
#include "Flare/FlareAssert.h"
#include "FlareNativeConfig.h"

#define STBI_ASSERT(x) FLARE_ASSERT_MSG(x, "STBI Assert")

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

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