#include <stdio.h>

#include "FlashFireNativeConfig.h"
#include "Application.h"

constexpr char ExecDir[] = "./FlashFireCS.dll";

int main(int a_argc, char* a_argv[])
{
    printf("FlashFire %d.%d \n", FLASHFIRENATIVE_VERSION_MAJOR, FLASHFIRENATIVE_VERSION_MINOR);

    Application app;
    app.Run((int32_t)a_argc, a_argv);

    return 0;
}