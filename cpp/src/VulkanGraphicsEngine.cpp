#include "Rendering/Vulkan/VulkanGraphicsEngine.h"

#include <future>
#include <mutex>

#include "FlareAssert.h"
#include "Logger.h"
#include "ObjectManager.h"
#include "Profiler.h"
#include "Rendering/Light.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/ShaderBuffers.h"
#include "Rendering/Vulkan/VulkanGraphicsEngineBindings.h"
#include "Rendering/Vulkan/VulkanModel.h"
#include "Rendering/Vulkan/VulkanPipeline.h"
#include "Rendering/Vulkan/VulkanPixelShader.h"
#include "Rendering/Vulkan/VulkanRenderCommand.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanRenderTexture.h"
#include "Rendering/Vulkan/VulkanShaderData.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"
#include "Rendering/Vulkan/VulkanTextureSampler.h"
#include "Rendering/Vulkan/VulkanVertexShader.h"
#include "Runtime/RuntimeFunction.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

VulkanGraphicsEngine::VulkanGraphicsEngine(RuntimeManager* a_runtime, VulkanRenderEngineBackend* a_vulkanEngine)
{
    m_vulkanEngine = a_vulkanEngine;
    m_runtimeManager = a_runtime;

    TRACE("Creating Vulkan Command Pool");
    vk::Device device = m_vulkanEngine->GetLogicalDevice();

    const vk::CommandPoolCreateInfo poolInfo = vk::CommandPoolCreateInfo
    (
        vk::CommandPoolCreateFlagBits::eTransient,
        m_vulkanEngine->GetGraphicsQueueIndex()
    );  

    if (device.createCommandPool(&poolInfo, nullptr, &m_commandPool) != vk::Result::eSuccess)
    {
        Logger::Error("Failed to create command pool");

        assert(0);
    }

    m_runtimeBindings = new VulkanGraphicsEngineBindings(m_runtimeManager, this);

    m_preShadowFunc = m_runtimeManager->GetFunction("FlareEngine.Rendering", "RenderPipeline", ":PreShadowS(uint)");
    m_postShadowFunc = m_runtimeManager->GetFunction("FlareEngine.Rendering", "RenderPipeline", ":PostShadowS(uint)");
    m_preRenderFunc = m_runtimeManager->GetFunction("FlareEngine.Rendering", "RenderPipeline", ":PreRenderS(uint)");
    m_postRenderFunc = m_runtimeManager->GetFunction("FlareEngine.Rendering", "RenderPipeline", ":PostRenderS(uint)");
    m_preLightFunc = m_runtimeManager->GetFunction("FlareEngine.Rendering", "RenderPipeline", ":PreLightS(uint,uint)");
    m_postLightFunc = m_runtimeManager->GetFunction("FlareEngine.Rendering", "RenderPipeline", ":PostLightS(uint,uint)");
    m_postProcessFunc = m_runtimeManager->GetFunction("FlareEngine.Rendering", "RenderPipeline", ":PostProcessS(uint)"); 
}
VulkanGraphicsEngine::~VulkanGraphicsEngine()
{
    delete m_runtimeBindings;

    delete m_preShadowFunc;
    delete m_postShadowFunc;
    delete m_preRenderFunc;
    delete m_postRenderFunc;
    delete m_preLightFunc;
    delete m_postLightFunc;
    delete m_postProcessFunc;

    const vk::Device device = m_vulkanEngine->GetLogicalDevice();

    TRACE("Deleting command pool");
    device.destroyCommandPool(m_commandPool);

    TRACE("Deleting Pipelines");
    for (const auto& iter : m_pipelines)
    {
        delete iter.second;
    }

    TRACE("Checking if shaders where deleted");
    for (uint32_t i = 0; i < m_vertexShaders.Size(); ++i)
    {
        if (m_vertexShaders[i] != nullptr)
        {
            Logger::Warning("Vertex Shader was not destroyed");

            delete m_vertexShaders[i];
        }
    }

    for (uint32_t i = 0; i < m_pixelShaders.Size(); ++i)
    {
        if (m_pixelShaders[i] != nullptr)
        {
            Logger::Warning("Pixel Shader was not destroyed");

            delete m_pixelShaders[i];
        }
    }

    TRACE("Checking if models where deleted");
    for (uint32_t i = 0; i < m_models.Size(); ++i)
    {
        if (m_models[i] != nullptr)
        {
            Logger::Warning("Model was not destroyed");

            delete m_models[i];
            m_models[i] = nullptr;
        }
    }

    TRACE("Checking camera buffer health");
    if (m_cameraBuffers.Size() != 0)
    {
        Logger::Warning("Leaked Camera Buffer");
    }

    TRACE("Checking shader program buffer health");
    for (uint32_t i = 0; i < m_shaderPrograms.Size(); ++i)
    {
        if (!(m_shaderPrograms[i].Flags & 0b1 << RenderProgram::FreeFlag))
        {
            Logger::Warning("Shader buffer was not destroyed");
        }

        if (m_shaderPrograms[i].Data != nullptr)
        {
            Logger::Warning("Shader data was not destroyed");

            delete (VulkanShaderData*)m_shaderPrograms[i].Data;
            m_shaderPrograms[i].Data = nullptr;
        }
    }

    TRACE("Checking if render textures where deleted");
    for (uint32_t i = 0; i < m_renderTextures.Size(); ++i)
    {
        if (m_renderTextures[i] != nullptr)
        {
            Logger::Warning("Render Texture was not destroyed");

            delete m_renderTextures[i];
            m_renderTextures[i] = nullptr;
        }
    }

    TRACE("Checking if texture samplers where deleted");
    for (uint32_t i = 0; i < m_textureSampler.Size(); ++i)
    {
        if (m_textureSampler[i].TextureMode != TextureMode_Null)
        {
            Logger::Warning("Texture sampler was not destroyed");
        }

        if (m_textureSampler[i].Data != nullptr)
        {
            Logger::Warning("Texture sampler data was not destroyed");

            delete (VulkanTextureSampler*)m_textureSampler[i].Data;
            m_textureSampler[i].Data = nullptr;
        }
    }
}

RenderProgram VulkanGraphicsEngine::GetRenderProgram(uint32_t a_addr)
{
    FLARE_ASSERT_MSG(a_addr < m_shaderPrograms.Size(), "GetRenderProgram out of bounds");

    return m_shaderPrograms[a_addr];
}
VulkanPipeline* VulkanGraphicsEngine::GetPipeline(uint32_t a_renderTexture, uint32_t a_pipeline)
{
    const uint64_t addr = (uint64_t)a_renderTexture | (uint64_t)a_pipeline << 32;

    {
        const std::lock_guard g = std::lock_guard(m_pipeLock);
        auto iter = m_pipelines.find(addr);
        if (iter != m_pipelines.end())
        {
            return iter->second;
        }
    }

    TRACE("Creating Vulkan Pipeline");
    const VulkanRenderTexture* tex = GetRenderTexture(a_renderTexture);

    FLARE_ASSERT_MSG(a_pipeline < m_shaderPrograms.Size(), "GetPipeline pipeline out of bounds");

    vk::RenderPass pass = m_swapchain->GetRenderPass();
    bool hasDepth = false;
    uint32_t textureCount = 1;

    if (tex != nullptr)
    {
        pass = tex->GetRenderPass();
        hasDepth = tex->HasDepthTexture();
        textureCount = tex->GetTextureCount();
    }

    VulkanPipeline* pipeline = new VulkanPipeline(m_vulkanEngine, this, pass, hasDepth, textureCount, a_pipeline);
    {
        const std::lock_guard g = std::lock_guard(m_pipeLock);
        m_pipelines.emplace(addr, pipeline);
    }

    return pipeline;
}

static glm::ivec2 SetViewport(VulkanRenderCommand& a_renderCommand, const CameraBuffer& a_buffer, VulkanSwapchain* a_swapchain, vk::CommandBuffer a_commandBuffer)
{
    glm::ivec2 renderSize = a_swapchain->GetSize();
    const VulkanRenderTexture* renderTexture = a_renderCommand.GetRenderTexture();
    if (renderTexture != nullptr)
    {
        renderSize = glm::ivec2((int)renderTexture->GetWidth(), (int)renderTexture->GetHeight());
    }

    const glm::vec2 screenPos = a_buffer.View.Position * (glm::vec2)renderSize;
    const glm::vec2 screenSize = a_buffer.View.Size * (glm::vec2)renderSize;

    const vk::Rect2D scissor = vk::Rect2D({ (int32_t)screenPos.x, (int32_t)screenPos.y }, { (uint32_t)screenSize.x, (uint32_t)screenSize.y });
    a_commandBuffer.setScissor(0, 1, &scissor);

    const vk::Viewport viewport = vk::Viewport
    (
        screenPos.x,
        screenPos.y,
        screenSize.x,
        screenSize.y,
        a_buffer.View.MinDepth,
        a_buffer.View.MaxDepth
    );
    a_commandBuffer.setViewport(0, 1, &viewport);

    return renderSize;
}

vk::CommandBuffer VulkanGraphicsEngine::DrawCamera(uint32_t a_camIndex) 
{
    // While there is no code relating to mono in here for now.
    // This is used to fix a crash relating to locking a Thread after going from Mono -> Native 
    // When when another thread tries to aquire a lock after and it is not visible from Mono despite still being native will cause MemMap Crash 
    // 
    // ^ No longer applicable as actually using mono functions on this thread however leaving for future reference and reminder to attach all threads
    m_runtimeManager->AttachThread();
    
    const vk::Device device = m_vulkanEngine->GetLogicalDevice();
    const uint32_t curFrame = m_vulkanEngine->GetCurrentFrame();
    const RenderEngine* renderEngine = m_vulkanEngine->GetRenderEngine();
    ObjectManager* objectManager = renderEngine->GetObjectManager();

    const CameraBuffer& camBuffer = m_cameraBuffers[a_camIndex];

    const vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo
    (
        m_commandPool,
        vk::CommandBufferLevel::ePrimary,
        1
    );

    vk::CommandBuffer commandBuffer;
    if (device.allocateCommandBuffers(&allocInfo, &commandBuffer) != vk::Result::eSuccess)
    {
        Logger::Error("Failed to Allocate Command Buffers");

        assert(0);
    }

    constexpr vk::CommandBufferBeginInfo BeginInfo = vk::CommandBufferBeginInfo
    (
        vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    );
    commandBuffer.begin(BeginInfo);

    VulkanRenderCommand& renderCommand = m_renderCommands.Push(VulkanRenderCommand(m_vulkanEngine, this, m_swapchain, commandBuffer));

    void* camArgs[] = 
    { 
        &a_camIndex 
    };

    m_preRenderFunc->Exec(camArgs);

    glm::vec2 screenSize = SetViewport(renderCommand, camBuffer, m_swapchain, commandBuffer);    

    const std::vector<MaterialRenderStack> stacks = m_renderStacks.ToVector();

    for (const MaterialRenderStack& renderStack : stacks)
    {
        const uint32_t matAddr = renderStack.GetMaterialAddr();
        const RenderProgram& program = m_shaderPrograms[matAddr];
        if (camBuffer.RenderLayer & program.RenderLayer)
        {
            const VulkanPipeline* pipeline = renderCommand.BindMaterial(matAddr);
            FLARE_ASSERT(pipeline != nullptr);
            const VulkanShaderData* shaderData = (VulkanShaderData*)program.Data;
            FLARE_ASSERT(shaderData != nullptr);
            shaderData->UpdateCameraBuffer(curFrame, screenSize, camBuffer, objectManager);
            
            const std::vector<ModelBuffer> modelBuffers = renderStack.GetModelBuffers();
            for (const ModelBuffer& modelBuff : modelBuffers)
            {
                if (modelBuff.ModelAddr != -1)
                {
                    VulkanModel* model = m_models[modelBuff.ModelAddr];
                    if (model != nullptr)
                    {
                        const std::lock_guard mLock = std::lock_guard(model->GetLock());
                        model->Bind(commandBuffer);
                        const uint32_t indexCount = model->GetIndexCount();
                        for (uint32_t tAddr : modelBuff.TransformAddr)
                        {
                            shaderData->UpdateTransformBuffer(commandBuffer, curFrame, tAddr, objectManager);

                            commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
                        }
                    }
                }
            }
        }
    }
    
    m_postRenderFunc->Exec(camArgs);

    renderCommand.BindRenderTexture(camBuffer.RenderTextureAddr);
    screenSize = SetViewport(renderCommand, camBuffer, m_swapchain, commandBuffer);

    for (uint32_t i = 0; i < 1; ++i)
    {
        void* lightArgs[] = 
        {
            &i,
            &a_camIndex
        };

        m_preLightFunc->Exec(lightArgs);

        const VulkanPipeline* pipeline = renderCommand.GetPipeline();
        if (pipeline == nullptr)
        {
            continue;
        }

        const VulkanShaderData* data = pipeline->GetShaderData();
        FLARE_ASSERT(data != nullptr);
        data->UpdateCameraBuffer(curFrame, screenSize, camBuffer, objectManager);

        switch ((e_LightType)i)
        {
        case LightType_Directional:
        {
            const std::vector<DirectionalLightBuffer> lights = m_directionalLights.ToVector();

            for (const DirectionalLightBuffer& dirLight : lights)
            {
                if (dirLight.TransformAddr != -1 && camBuffer.RenderLayer & dirLight.RenderLayer)
                {
                    commandBuffer.draw(4, 1, 0, 0);
                }
            }

            break;
        }
        case LightType_Point:
        {
            break;
        }
        case LightType_Spot:
        {
            break;
        }
        default:
        {
            Logger::Warning("FlareEngine: Invalid light type when drawing");

            break;
        }
        }

        m_postLightFunc->Exec(lightArgs);
    }

    m_postProcessFunc->Exec(camArgs);

    renderCommand.Flush();
    
    commandBuffer.end();

    return commandBuffer;
}

std::vector<vk::CommandBuffer> VulkanGraphicsEngine::Update()
{
    m_renderCommands.Clear();

    const uint32_t camBufferSize = m_cameraBuffers.Size();
    std::vector<vk::CommandBuffer> cmdBuffers;
    std::vector<std::future<vk::CommandBuffer>> futures;
    for (uint32_t i = 0; i < camBufferSize; ++i)
    {
        futures.emplace_back(std::async(std::bind(&VulkanGraphicsEngine::DrawCamera, this, i)));
    }

    for (std::future<vk::CommandBuffer>& f : futures)
    {
        f.wait();
        cmdBuffers.emplace_back(f.get());
    }

    return cmdBuffers;
}

VulkanVertexShader* VulkanGraphicsEngine::GetVertexShader(uint32_t a_addr)
{
    FLARE_ASSERT_MSG(a_addr < m_vertexShaders.Size(), "GetVertexShader out of bounds");

    return m_vertexShaders[a_addr];
}
VulkanPixelShader* VulkanGraphicsEngine::GetPixelShader(uint32_t a_addr)
{
    FLARE_ASSERT_MSG(a_addr < m_pixelShaders.Size(), "GetPixelShader out of bounds");

    return m_pixelShaders[a_addr];
}

VulkanRenderTexture* VulkanGraphicsEngine::GetRenderTexture(uint32_t a_addr)
{
    if (a_addr == -1)
    {
        return nullptr;
    }

    FLARE_ASSERT_MSG(a_addr < m_renderTextures.Size(), "GetRenderTexture out of bounds");

    return m_renderTextures[a_addr];
}