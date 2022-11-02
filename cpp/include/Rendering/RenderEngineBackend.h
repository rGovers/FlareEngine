#pragma once

#include <string_view>

class RenderEngine;

class RenderEngineBackend
{
private:

protected:
    RenderEngine* m_renderEngine;

public:
    RenderEngineBackend(RenderEngine* a_engine) 
    {
        m_renderEngine = a_engine;
    }
    virtual ~RenderEngineBackend() { }

    virtual void Update() = 0;
};