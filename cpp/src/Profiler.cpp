#include "Profiler.h"

#include <cassert>
#include <mutex>

#include "Logger.h"
#include "Trace.h"

Profiler* Profiler::Instance = nullptr;
Profiler::Callback* Profiler::CallbackFunc = nullptr;

Profiler::Profiler()
{

}
Profiler::~Profiler()
{
    for (auto iter = m_data.begin(); iter != m_data.end(); ++iter)
    {
        delete iter->second;
    }
}

void Profiler::Init()
{
#ifdef FLARENATIVE_ENABLE_PROFILER
    TRACE("Initializing Profiler");
    if (Instance == nullptr)
    {
        Instance = new Profiler();
    }
#endif
}
void Profiler::Destroy()
{
#ifdef FLARENATIVE_ENABLE_PROFILER
    TRACE("Destroying Profiler");
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
#endif
}

void Profiler::Start(const std::string_view& a_name)
{
#ifdef FLARENATIVE_ENABLE_PROFILER
    const std::unique_lock lock = std::unique_lock(Instance->m_mutex);

    const std::thread::id tID = std::this_thread::get_id();

    auto iter = Instance->m_data.find(tID);

    if (iter != Instance->m_data.end())
    {
        iter->second->Frames.clear();
    }
    else
    {
        PData* data = new PData();
        data->Name = std::string(a_name);

        Instance->m_data.emplace(tID, data);
    }
#endif
}
void Profiler::Stop()
{
#ifdef FLARENATIVE_ENABLE_PROFILER
    if (CallbackFunc != nullptr)
    {
        const std::shared_lock lock = std::shared_lock(Instance->m_mutex);

        const std::thread::id tID = std::this_thread::get_id();

        const auto iter = Instance->m_data.find(tID);
        if (iter == Instance->m_data.end())
        {
            Logger::Error("FlareEngine: Profiler not started on thread");

            assert(0);
        }

        (*CallbackFunc)(*iter->second);
    }
#endif
}

void Profiler::StartFrame(const std::string_view& a_name)
{
#ifdef FLARENATIVE_ENABLE_PROFILER
    const std::shared_lock lock = std::shared_lock(Instance->m_mutex);

    const std::thread::id tID = std::this_thread::get_id();

    const auto iter = Instance->m_data.find(tID);
    if (iter == Instance->m_data.end())
    {
        Logger::Error("FlareEngine: Profiler not started on thread");

        assert(0);
    }

    ProfileFrame frame;
    frame.StartTime = std::chrono::high_resolution_clock::now();
    frame.EndTime = frame.StartTime;
    frame.Name = std::string(a_name);
    frame.Stack = 0;
    if (!iter->second->Frames.empty())
    {
        auto iIter = iter->second->Frames.end();
        while (iIter != iter->second->Frames.begin())
        {
            --iIter;

            if (iIter->EndTime == iIter->StartTime)
            {
                frame.Stack = iIter->Stack + 1;

                break;
            }
        }
    }

    iter->second->Frames.emplace_back(frame);
#endif
}
void Profiler::StopFrame()
{
#ifdef FLARENATIVE_ENABLE_PROFILER
    const std::shared_lock lock = std::shared_lock(Instance->m_mutex);

    const std::thread::id tID = std::this_thread::get_id();

    const auto iter = Instance->m_data.find(tID);
    if (iter == Instance->m_data.end())
    {
        Logger::Error("FlareEngine: Profiler not started on thread");

        assert(0);
    }

    if (iter->second->Frames.empty())
    {
        Logger::Error("FlareEngine: Profiler Frame not created on thread");
    }

    auto iIter = iter->second->Frames.end();
    while (iIter != iter->second->Frames.begin())
    {
        --iIter;

        if (iIter->EndTime == iIter->StartTime)
        {
            iIter->EndTime = std::chrono::high_resolution_clock::now();

            return;
        }
    }

    Logger::Error("FlareEngine: Profile Start End Frame mismatch");
#endif
}