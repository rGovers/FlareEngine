#pragma once

#include "AppWindow/AppWindow.h"

#include <cstdint>

#include "PipeMessage.h"

class HeadlessAppWindow : public AppWindow
{
private:
    int      m_sock;
    
    bool     m_close;

    uint32_t m_width;
    uint32_t m_height;

    PipeMessage RecieveMessage() const;

protected:

public:
    HeadlessAppWindow();
    ~HeadlessAppWindow();

    virtual bool ShouldClose() const;

    virtual double GetDelta() const;
    virtual double GetTime() const;

    virtual void Update();
};