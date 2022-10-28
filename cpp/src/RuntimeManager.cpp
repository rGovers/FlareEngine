#include "RuntimeManager.h"

#include <assert.h>

#include <mono/metadata/mono-config.h>

RuntimeManager::RuntimeManager()
{
    mono_config_parse(NULL);
    
    m_domain = mono_jit_init_version("Core", "v4.0");
    m_assembly = mono_domain_assembly_open(m_domain, "FlareCS.dll");
    assert(m_assembly != nullptr);
}
RuntimeManager::~RuntimeManager()
{
    mono_jit_cleanup(m_domain);
}

void RuntimeManager::Exec(int a_argc, char* a_argv[])
{
    const int retVal = mono_jit_exec(m_domain, m_assembly, a_argc, a_argv);
    assert(retVal == 0);
}