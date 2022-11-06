#pragma once

#include <stdio.h>

#ifndef NDEBUG
#define TRACE(str) printf("FlareEngine: "); printf(str); printf("\n");
#elif defined FLARENATIVE_ENABLE_TRACE
#define TRACE(str) printf("FlareEngine: "); printf(str); printf("\n");
#else
#define TRACE(str)
#endif