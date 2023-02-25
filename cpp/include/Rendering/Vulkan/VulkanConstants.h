#pragma once

#include <cstdint>

static constexpr uint32_t VulkanMaxFlightFrames = 2;
static constexpr uint32_t VulkanFlightPoolSize = VulkanMaxFlightFrames + 1;

#ifdef NDEBUG
static constexpr bool VulkanEnableValidationLayers = false;
#else
static constexpr bool VulkanEnableValidationLayers = true;
#endif