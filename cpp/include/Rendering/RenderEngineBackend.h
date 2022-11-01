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

    virtual uint32_t GenerateVertexShaderAddr(const std::string_view& a_str) = 0;
    virtual void DestoryVertexShader(uint32_t a_addr) = 0;

    virtual uint32_t GeneratePixelShaderAddr(const std::string_view& a_str) = 0;
    virtual void DestroyPixelShader(uint32_t a_addr) = 0;
};