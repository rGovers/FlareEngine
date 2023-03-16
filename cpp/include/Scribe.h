#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "FlareAssert.h"

class RuntimeManager;

class Scribe
{
private:
    static Scribe* Instance;

    std::string                                     m_curLang;
    std::unordered_map<std::string, std::u32string> m_strings;

    Scribe();

protected:

public:
    ~Scribe();

    static void Init(RuntimeManager* a_runtime);
    static void Destroy();

    inline static std::string GetCurrentLanguage()
    {
        FLARE_ASSERT(Instance != nullptr);

        return Instance->m_curLang;
    }
    inline static void SetCurrentLanguage(const std::string_view& a_language)
    {
        FLARE_ASSERT(Instance != nullptr);

        Instance->m_curLang = std::string(a_language);
    }

    inline static bool KeyExists(const std::string_view& a_key)
    {
        return Instance->m_strings.find(std::string(a_key)) != Instance->m_strings.end();
    }

    static void SetString(const std::string_view& a_key, const std::u32string_view& a_string);

    static std::u32string GetString(const std::string_view& a_key);
    static std::u32string GetStringFormated(const std::string_view& a_key, char32_t* const* a_args, uint32_t a_count);
    static std::u32string GetStringFormated(const std::string_view& a_key, const std::u32string* a_args, uint32_t a_count);
};