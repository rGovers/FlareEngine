#pragma once

#include "AppWindow/AppWindow.h"

#if WIN32
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

class HeadlessAppWindow : public AppWindow
{
private:
    static constexpr std::string_view PipeName = "FlareEngine-IPC";

#if WIN32
    SOCKET                                         m_sock;
#else
    int                                            m_sock;
#endif

    bool                                           m_unlockWindow;    
    bool                                           m_close;

    std::mutex                                     m_fLock;

    TArray<PipeMessage>                            m_queuedMessages;

    char*                                          m_frameData;

    uint32_t                                       m_width;
    uint32_t                                       m_height;

    std::chrono::high_resolution_clock::time_point m_prevTime;
   
    double                                         m_delta;
    double                                         m_time;

    PipeMessage ReceiveMessage() const;
    void PushMessage(const PipeMessage& a_msg) const;

    void MessageCallback(const std::string_view& a_message, e_LoggerMessageType a_type);

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
    virtual std::vector<const char*> GetRequiredVulkanExtenions() const
    {
        return std::vector<const char*>();
    }
    virtual vk::SurfaceKHR GetSurface(const vk::Instance& a_instance)
    {
        return vk::SurfaceKHR();
    }

    void PushFrameData(uint32_t a_width, uint32_t a_height, const char* a_buffer);
};