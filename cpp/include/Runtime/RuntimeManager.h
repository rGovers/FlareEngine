#pragma once

#include <cstdint>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <string_view>

class RenderEngine;
class RuntimeFunction;

#if WIN32
#define FLARE_MONO_EXPORT(ret, func, ...) __declspec(dllexport) ret func(__VA_ARGS__)
#else
#define FLARE_MONO_EXPORT(ret, func, ...) static ret func(__VA_ARGS__)
#endif

class RuntimeManager
{
private:
    MonoDomain*   m_domain;
    MonoAssembly* m_assembly;

    MonoImage*    m_image;

    MonoClass*    m_programClass;

    MonoMethod*   m_updateMethod;
    MonoMethod*   m_shutdownMethod;

protected:

public:
    RuntimeManager();
    ~RuntimeManager();

    void BindFunction(const std::string_view& a_location, void* a_function);

    void Exec(int32_t a_argc, char* a_argv[]);
    void Update(double a_delta, double a_time);

    void AttachThread();

    RuntimeFunction* GetFunction(const std::string_view& a_namespace, const std::string_view& a_class, const std::string_view& a_method) const;
};