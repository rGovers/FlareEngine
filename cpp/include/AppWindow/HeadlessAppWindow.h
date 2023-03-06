#pragma once

#include "AppWindow/AppWindow.h"

#include "FlarePlatform.h"

#ifdef FLARE_WINDOWS
#include <WinSock2.h>
#include <Windows.h>
#include <afunix.h>
#endif

#include <chrono>
#include <cstdint>
#include <mutex>

#include "DataTypes/TArray.h"
#include "Logger.h"
#include "PipeMessage.h"
#include "Profiler.h"

class HeadlessAppWindow : public AppWindow
{
private:
    static constexpr int NameMax = 16;
    static constexpr int FrameMax = 64;

    struct ProfileTFrame
    {
        char Name[NameMax];
        float Time;
        uint8_t Stack;
    };

    struct ProfileScope
    {
        char Name[NameMax];
        uint16_t FrameCount;
        ProfileTFrame Frames[FrameMax];
    };

    static constexpr std::string_view PipeName = "FlareEngine-IPC";

#ifdef FLARE_WINDOWS
    SOCKET                                         m_sock;

    char*                                          m_frameData;
    volatile bool                                  m_unlockWindow;    
#else
    int                                            m_sock;
#endif

    bool                                           m_close;

    std::mutex                                     m_fLock;

    TArray<PipeMessage>                            m_queuedMessages;


    uint32_t                                       m_width;
    uint32_t                                       m_height;

    std::chrono::high_resolution_clock::time_point m_prevTime;
   
    double                                         m_delta;
    double                                         m_time;

    void PushMessageQueue();

    PipeMessage ReceiveMessage() const;
    void PushMessage(const PipeMessage& a_msg) const;

    void MessageCallback(const std::string_view& a_message, e_LoggerMessageType a_type);
    void ProfilerCallback(const Profiler::PData& a_profilerData);

    bool PollMessage();

protected:

public:
    HeadlessAppWindow();
    ~HeadlessAppWindow();

    virtual bool ShouldClose() const;

    virtual double GetDelta() const;
    virtual double GetTime() const;

    virtual void Update();

    virtual glm::ivec2 GetSize() const;

    virtual bool IsHeadless() const
    {
        return true;
    }

    virtual vk::SurfaceKHR GetSurface(const vk::Instance& a_instance)
    {
        return vk::SurfaceKHR(nullptr);
    }
    virtual std::vector<const char*> GetRequiredVulkanExtenions() const;
    virtual std::vector<const char*> GetRequiredVulkanDeviceExtensions() const;

#ifdef FLARE_WINDOWS
    void PushFrameData(uint32_t a_width, uint32_t a_height, const char* a_buffer, double a_delta, double a_time);
#else
    void PushTextureHandle(int a_fd, uint64_t a_size, int32_t a_slot, uint64_t a_offset);
#endif
};