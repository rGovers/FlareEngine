#pragma once

#include <string_view>

enum e_RenderingEngine
{
    RenderingEngine_Null,
    RenderingEngine_Vulkan
};

class AppWindow;
class Config;
class ObjectManager;
class RenderEngineBackend;
class RuntimeManager;

class RenderEngine
{
private:
    friend class VulkanRenderEngineBackend;

    Config*              m_config;

    ObjectManager*       m_objectManager;

    RenderEngineBackend* m_backend;

    AppWindow*           m_window;

protected:

public:
    RenderEngine(RuntimeManager* a_runtime, ObjectManager* a_objectManager, AppWindow* a_window, Config* a_config);
    ~RenderEngine();

    void Update();

    inline ObjectManager* GetObjectManager() const
    {
        return m_objectManager;
    }
};