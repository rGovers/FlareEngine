#pragma once

#include <string_view>

class RenderEngine;

class RenderEngineBackend
{
private:
    RenderEngine* m_renderEngine;
    
protected:

public:
    RenderEngineBackend(RenderEngine* a_engine) 
    {
        m_renderEngine = a_engine;
    }
    virtual ~RenderEngineBackend() { }

    inline RenderEngine* GetRenderEngine() const
    {
        return m_renderEngine;
    }

    virtual void Update() = 0;
};