#pragma once

#include <cstdint>

static constexpr uint32_t VulkanMaxFlightFrames = 2;

#ifdef NDEBUG
static constexpr bool VulkanEnableValidationLayers = false;
#else
static constexpr bool VulkanEnableValidationLayers = true;
#endif