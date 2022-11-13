#pragma once

#include "AppWindow/AppWindow.h"

#include <cstdint>

#include "PipeMessage.h"

class HeadlessAppWindow : public AppWindow
{
private:
    int      m_sock;
    
    bool     m_close;

    char*    m_frameData;

    uint32_t m_width;
    uint32_t m_height;

    PipeMessage RecieveMessage() const;
    void PushMessage(const PipeMessage& a_msg) const;

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