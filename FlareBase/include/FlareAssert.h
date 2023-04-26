#pragma once

#ifndef NDEBUG
#define FLARE_ENABLE_ASSERT
#endif

#include <cassert>

#include "Logger.h"

#ifdef FLARE_ENABLE_ASSERT
#define FLARE_ASSERT(val) if (!(val)) { Logger::Error("FlareAssert: " #val); assert(0); }
#define FLARE_ASSERT_R(val) if (!(val)) { Logger::Error("FlareAssert: " #val); assert(0); }
#define FLARE_ASSERT_MSG(val, msg) if (!(val)) { Logger::Error(std::string("FlareAssert: ") + (msg) + ": " #val); assert(0); }
#define FLARE_ASSERT_MSG_R(val, msg) if (!(val)) { Logger::Error(std::string("FlareAssert: ") + (msg) + ": " #val); assert(0); }
#else
#define FLARE_ASSERT void(0);
#define FLARE_ASSERT_R(val) if (!(val)) { Logger::Error("FlareAssert: " #val); }
#define FLARE_ASSERT_MSG(val, msg) void(0);
#define FLARE_ASSERT_MSG_R(val, msg) if (!(val)) { Logger::Error(std::string("FlareAssert: ") + (msg) + ": " #val); }
#endif