#pragma once

#include <cstdint>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

class RenderEngine;

class RuntimeManager
{
private:
    MonoDomain*   m_domain;
    MonoAssembly* m_assembly;

    MonoClass*    m_programClass;

    MonoMethod*   m_updateMethod;
    MonoMethod*   m_shutdownMethod;

protected:

public:
    RuntimeManager(RenderEngine* a_renderEngine);
    ~RuntimeManager();

    void Exec(int32_t a_argc, char* a_argv[]);
    void Update(double a_delta, double a_time);
};