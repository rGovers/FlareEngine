#pragma once

#include <cstdint>

class Config;
class RuntimeManager;

class Application
{
private:
    Config*         m_config;
    RuntimeManager* m_runtime;

protected:

public:
    Application();
    ~Application();

    void Run(int32_t a_argc, char* a_argv[]);
};