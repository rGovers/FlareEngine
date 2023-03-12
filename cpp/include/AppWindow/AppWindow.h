#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

class Application;

class AppWindow
{
private:
    Application* m_app;

protected:

public:
    AppWindow(Application* a_app)
    {
        m_app = a_app;
    }
    virtual ~AppWindow() { }

    virtual bool ShouldClose() const = 0;

    virtual double GetDelta() const = 0;
    virtual double GetTime() const = 0;

    virtual void Update() = 0;

    inline Application* GetApplication() const
    {
        return m_app;
    }

    virtual glm::ivec2 GetSize() const = 0;

    virtual bool IsHeadless() const = 0;

    virtual std::vector<const char*> GetRequiredVulkanExtenions() const = 0;
    virtual vk::SurfaceKHR GetSurface(const vk::Instance& a_instance) = 0;
};