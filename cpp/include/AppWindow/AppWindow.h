#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

class AppWindow
{
private:

protected:

public:
    AppWindow() { }
    virtual ~AppWindow() { }

    virtual bool ShouldClose() const = 0;

    virtual double GetDelta() const = 0;
    virtual double GetTime() const = 0;

    virtual void Update() = 0;

    virtual glm::ivec2 GetSize() const = 0;

    virtual bool IsHeadless() const = 0;

    virtual std::vector<const char*> GetRequiredVulkanExtenions() const = 0;
    virtual std::vector<const char*> GetRequiredVulkanDeviceExtensions() const = 0;
    virtual vk::SurfaceKHR GetSurface(const vk::Instance& a_instance) = 0;
};