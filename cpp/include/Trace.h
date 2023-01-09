#pragma once

#include <stdio.h>

#ifndef NDEBUG
#define FLARENATIVE_ENABLE_TRACE
#endif

#ifdef FLARENATIVE_ENABLE_TRACE
#define TRACE(str) printf("FlareEngine: %s \n", str)
#else
#define TRACE(str) void(0)
#endif