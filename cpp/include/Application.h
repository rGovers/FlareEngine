#pragma once

#include <cstdint>

class AppWindow;
class Config;
class ObjectManager;
class RenderEngine;
class RuntimeManager;

class Application
{
private:
    AppWindow*      m_appWindow;

    Config*         m_config;
    ObjectManager*  m_objectManager;
    RuntimeManager* m_runtime;
    RenderEngine*   m_renderEngine;

protected:

public:
    Application(Config* a_config);
    ~Application();

    void Run(int32_t a_argc, char* a_argv[]);
};