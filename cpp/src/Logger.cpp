#include "Logger.h"

#include <iostream>

#include "RuntimeManager.h"
#include "Trace.h"

Logger::Callback* Logger::CallbackFunc = nullptr;

static void Logger_PushMessage(MonoString* a_string)
{
    char* str = mono_string_to_utf8(a_string);

    Logger::Message(str);

    mono_free(str);
}
static void Logger_PushWarning(MonoString* a_string)
{
    char* str = mono_string_to_utf8(a_string);

    Logger::Warning(str);

    mono_free(str);
}
static void Logger_PushError(MonoString* a_string)
{
    char* str = mono_string_to_utf8(a_string);

    Logger::Error(str);

    mono_free(str);
}

void Logger::Message(const std::string_view& a_msg)
{
    if (CallbackFunc != nullptr)
    {
        (*CallbackFunc)(a_msg, LoggerMessageType_Message);
    }
    else
    {
        std::cout << a_msg << "\n";
    }
}
void Logger::Warning(const std::string_view& a_msg)
{
    if (CallbackFunc != nullptr)
    {
        (*CallbackFunc)(a_msg, LoggerMessageType_Warning);
    }
    else
    {
        std::cout << a_msg << "\n";
    }
}
void Logger::Error(const std::string_view& a_msg)
{
    if (CallbackFunc != nullptr)
    {
        (*CallbackFunc)(a_msg, LoggerMessageType_Error);
    }
    else
    {
        std::cout << a_msg << "\n";
    }
}
void Logger::InitRuntime(RuntimeManager* a_runtime)
{
    TRACE("Initializing C# Logger");

    a_runtime->BindFunction("FlareEngine.Logger::PushMessage", (void*)Logger_PushMessage);
    a_runtime->BindFunction("FlareEngine.Logger::PushWarning", (void*)Logger_PushWarning);
    a_runtime->BindFunction("FlareEngine.Logger::PushError", (void*)Logger_PushError);
}