#include "RuntimeManager.h"

#include <assert.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-config.h>

#include "Rendering/RenderEngine.h"

RuntimeManager::RuntimeManager()
{
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

void RuntimeManager::BindFunction(const std::string_view& a_location, void* a_function)
{
    mono_add_internal_call(a_location.begin(), a_function);
}