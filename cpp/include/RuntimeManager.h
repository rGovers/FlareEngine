#pragma once

#include <cstdint>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

class RuntimeManager
{
private:
    MonoDomain*   m_domain;
    MonoAssembly* m_assembly;

protected:

public:
    RuntimeManager();
    ~RuntimeManager();

    void Exec(int32_t a_argc, char* a_argv[]);
};