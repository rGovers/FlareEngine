#include "Scribe.h"

#include <codecvt>
#include <locale>

#include "Runtime/RuntimeManager.h"

Scribe* Scribe::Instance = nullptr;

static std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;

FLARE_MONO_EXPORT(void, RUNTIME_FUNCTION_NAME(Scribe, SetInternalLanguage), MonoString* a_language)
{
    char* language = mono_string_to_utf8(a_language);

    Scribe::SetCurrentLanguage(language);

    mono_free(language);
}
FLARE_MONO_EXPORT(MonoString*, RUNTIME_FUNCTION_NAME(Scribe, GetInternalLanguage))
{
    return mono_string_new_wrapper(Scribe::GetCurrentLanguage().c_str());
}

FLARE_MONO_EXPORT(MonoString*, RUNTIME_FUNCTION_NAME(Scribe, GetString), MonoString* a_key)
{
    char* key = mono_string_to_utf8(a_key);

    const std::u32string str = Scribe::GetString(key);
    
    MonoString* mStr = nullptr;
    if (!str.empty())
    {
        mStr = mono_string_from_utf32((mono_unichar4*)str.c_str());
    }

    mono_free(key);

    return mStr;
}
FLARE_MONO_EXPORT(MonoString*, RUNTIME_FUNCTION_NAME(Scribe, GetStringFormated), MonoString* a_key, MonoArray* a_args)
{
    const uintptr_t size = mono_array_length(a_args);

    char32_t** args = nullptr;
    if (size > 0)
    {
        args = new char32_t*[size];
        for (uintptr_t i = 0; i < size; ++i)
        {
            args[i] = (char32_t*)mono_string_to_utf32(mono_array_get(a_args, MonoString*, i));
        }
    }
    
    char* key = mono_string_to_utf8(a_key);

    const std::u32string str = Scribe::GetStringFormated(key, args, (uint32_t)size);

    if (args != nullptr)
    {
        for (uint32_t i = 0; i < size; ++i)
        {
            mono_free(args[i]);
        }

        delete[] args;
    }

    MonoString* mStr = nullptr;
    if (!str.empty())
    {
        mStr = mono_string_from_utf32((mono_unichar4*)str.c_str());
    }

    return mStr;
}

FLARE_MONO_EXPORT(void, RUNTIME_FUNCTION_NAME(Scribe, SetString), MonoString* a_key, MonoString* a_value)
{
    char* key = mono_string_to_utf8(a_key);
    char32_t* value = (char32_t*)mono_string_to_utf32(a_value);

    Scribe::SetString(key, value);

    mono_free(key);
    mono_free(value);
}

FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(Scribe, Exists), MonoString* a_key)
{
    char* key = mono_string_to_utf8(a_key);

    const bool exists = Scribe::KeyExists(key);

    mono_free(key);

    return (uint32_t)exists;
}

Scribe::Scribe()
{
    
}
Scribe::~Scribe()
{

}

void Scribe::Init(RuntimeManager* a_runtime)
{
    if (Instance == nullptr)
    {
        Instance = new Scribe();

        a_runtime->BindFunction(RUNTIME_FUNCTION_STRING(FlareEngine, Scribe, SetInternalLanguage), (void*)RUNTIME_FUNCTION_NAME(Scribe, SetInternalLanguage));
        a_runtime->BindFunction(RUNTIME_FUNCTION_STRING(FlareEngine, Scribe, GetInternalLanguage), (void*)RUNTIME_FUNCTION_NAME(Scribe, GetInternalLanguage));
        a_runtime->BindFunction(RUNTIME_FUNCTION_STRING(FlareEngine, Scribe, GetString), (void*)RUNTIME_FUNCTION_NAME(Scribe, GetString));
        a_runtime->BindFunction(RUNTIME_FUNCTION_STRING(FlareEngine, Scribe, GetStringFormated), (void*)RUNTIME_FUNCTION_NAME(Scribe, GetStringFormated));
        a_runtime->BindFunction(RUNTIME_FUNCTION_STRING(FlareEngine, Scribe, SetString), (void*)RUNTIME_FUNCTION_NAME(Scribe, SetString));
        a_runtime->BindFunction(RUNTIME_FUNCTION_STRING(FlareEngine, Scribe, Exists), (void*)RUNTIME_FUNCTION_NAME(Scribe, Exists));
    }
}
void Scribe::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

void Scribe::SetString(const std::string_view& a_key, const std::u32string_view& a_string)
{
    FLARE_ASSERT(Instance != nullptr);

    const std::string key = std::string(a_key);

    const auto iter = Instance->m_strings.find(key);
    if (iter != Instance->m_strings.end())
    {
        iter->second = std::u32string(a_string);

        return;
    }

    Instance->m_strings.emplace(key, std::u32string(a_string));
}

std::u32string Scribe::GetString(const std::string_view& a_key)
{
    FLARE_ASSERT(Instance != nullptr);
        
    const std::string key = std::string(a_key);

    const auto iter = Instance->m_strings.find(key);
    if (iter != Instance->m_strings.end())
    {
        return iter->second;
    }

    return converter.from_bytes(key);
}
std::u32string Scribe::GetStringFormated(const std::string_view& a_key, char32_t* const* a_args, uint32_t a_count)
{
    std::u32string str = GetString(a_key);

    if (str.empty())
    {
        return str;
    }

    for (uint32_t i = 0; i < a_count; ++i)
    {
        const std::u32string iStr = converter.from_bytes("{" + std::to_string(i) + "}");
        const size_t size = iStr.size();

        while (true)
        {
            const std::size_t pos = str.find(iStr);
            if (pos == std::u32string::npos)
            {
                break;
            }

            str.replace(pos, size, a_args[i]);
        }   
    }

    return str;
}
std::u32string Scribe::GetStringFormated(const std::string_view& a_key, const std::u32string* a_args, uint32_t a_count)
{
    std::u32string str = GetString(a_key);

    if (str.empty())
    {
        return str;
    }

    for (uint32_t i = 0; i < a_count; ++i)
    {
        const std::u32string iStr = converter.from_bytes("{" + std::to_string(i) + "}");
        const size_t size = iStr.size();

        while (true)
        {
            const std::size_t pos = str.find(iStr);
            if (pos == std::u32string::npos)
            {
                break;
            }

            str.replace(pos, size, a_args[i]);
        }   
    }

    return str;
}