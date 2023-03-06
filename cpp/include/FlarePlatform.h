#pragma once

#if !defined(FLARE_WINDOWS) && !defined(FLARE_LINUX)
#if WIN32
#define FLARE_WINDOWS
#else
#define FLARE_LINUX
#endif
#endif