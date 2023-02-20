#pragma once

#ifndef NDEBUG
#define FLARENATIVE_ENABLE_ASSERT
#endif

#include <cassert>
#include "Logger.h"

#ifdef FLARENATIVE_ENABLE_ASSERT
#define FLARE_ASSERT(val) if (!(val)) { Logger::Error(std::string("FlareAssert: ") + #val); assert(0); }
#define FLARE_ASSERT_MSG(val, msg) if (!(val)) { Logger::Error(std::string("FlareAssert: ") + msg + ": " + #val); assert(0); }
#else
#define FLARE_ASSERT void(0);
#define FLARE_ASSERT_MSG(val, msg) void(0);
#endif