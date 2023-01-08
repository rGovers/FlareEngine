#pragma once

#include <stdio.h>

#ifndef NDEBUG
#define TRACE(str) printf("FlareEngine: %s \n", str);
#elif defined FLARENATIVE_ENABLE_TRACE
#define TRACE(str) printf("FlareEngine: %s \n", str);
#else
#define TRACE(str)
#endif