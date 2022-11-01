#include "RuntimeManager.h"

#include <assert.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-config.h>
#include <string_view>

#include "Rendering/RenderEngine.h"

static RenderEngine* REngine = nullptr;

static uint32_t VertexShader_GenerateShader(MonoString* a_string)
{
    char* str = mono_string_to_utf8(a_string);

    const uint32_t ret = REngine->GenerateVertexShaderAddr(str);

    free(str);

    return ret;
}
static void VertexShader_DestroyShader(uint32_t a_addr)
{
    REngine->DestroyVertexShader(a_addr);
}

static uint32_t PixelShader_GenerateShader(MonoString* a_string)
{
    char* str = mono_string_to_utf8(a_string);

    const uint32_t ret = REngine->GeneratePixelShaderAddr(str);

    free(str);

    return ret;
}
static void PixelShader_DestroyShader(uint32_t a_addr)
{
    REngine->DestroyPixelShader(a_addr);
}

RuntimeManager::RuntimeManager(RenderEngine* a_renderEngine)
{
    REngine = a_renderEngine;

    mono_config_parse(NULL);
    
    m_domain = mono_jit_init_version("Core", "v4.0");
    m_assembly = mono_domain_assembly_open(m_domain, "FlareCS.dll");
    assert(m_assembly != nullptr);

    MonoImage* image = mono_assembly_get_image(m_assembly);
    m_programClass = mono_class_from_name(image, "FlareEngine", "Program");

    MonoMethodDesc* updateDesc = mono_method_desc_new(":Update(double,double)", 0);
    m_updateMethod = mono_method_desc_search_in_class(updateDesc, m_programClass);

    MonoMethodDesc* shutdownDesc = mono_method_desc_new(":Shutdown()", 0);
    m_shutdownMethod = mono_method_desc_search_in_class(shutdownDesc, m_programClass);

    mono_method_desc_free(updateDesc);

    mono_add_internal_call("FlareEngine.Rendering.VertexShader::GenerateShader", (void*)VertexShader_GenerateShader);
    mono_add_internal_call("FlareEngine.Rendering.VertexShader::DestroyShader", (void*)VertexShader_DestroyShader);

    mono_add_internal_call("FlareEngine.Rendering.PixelShader::GenerateShader", (void*)PixelShader_GenerateShader);
    mono_add_internal_call("FlareEngine.Rendering.PixelShader::DestroyShader", (void*)PixelShader_DestroyShader);
}
RuntimeManager::~RuntimeManager()
{
    mono_runtime_invoke(m_shutdownMethod, nullptr, nullptr, nullptr);

    mono_jit_cleanup(m_domain);
}

void RuntimeManager::Exec(int a_argc, char* a_argv[])
{
    const int retVal = mono_jit_exec(m_domain, m_assembly, a_argc, a_argv);
    assert(retVal == 0);
}
void RuntimeManager::Update(double a_delta, double a_time)
{
    void* args[2];
    args[0] = &a_delta;
    args[1] = &a_time;

    mono_runtime_invoke(m_updateMethod, nullptr, args, nullptr);
}