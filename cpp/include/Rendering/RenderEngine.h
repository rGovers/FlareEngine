#pragma once

#include <string_view>
#include <thread>

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

    double               m_time;

    bool                 m_shutdown;
    bool                 m_join;
    std::thread          m_thread;

    Config*              m_config;

    ObjectManager*       m_objectManager;

    RenderEngineBackend* m_backend;

    AppWindow*           m_window;

    void Update(double a_delta, double a_time);
    void Run();
protected:

public:
    RenderEngine(RuntimeManager* a_runtime, ObjectManager* a_objectManager, AppWindow* a_window, Config* a_config);
    ~RenderEngine();

    void Start();
    void Stop();

    inline ObjectManager* GetObjectManager() const
    {
        return m_objectManager;
    }
};