#include "Rendering/Vulkan/VulkanGraphicsEngine.h"

#include <future>
#include <mutex>

#include "Logger.h"
#include "ObjectManager.h"
#include "Profiler.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/ShaderBuffers.h"
#include "Rendering/Vulkan/VulkanGraphicsEngineBindings.h"
#include "Rendering/Vulkan/VulkanModel.h"
#include "Rendering/Vulkan/VulkanPipeline.h"
#include "Rendering/Vulkan/VulkanPixelShader.h"
#include "Rendering/Vulkan/VulkanRenderCommand.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanRenderTexture.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"
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
    m_postProcessFunc = m_runtimeManager->GetFunction("FlareEngine.Rendering", "RenderPipeline", ":PostProcessS(uint)"); 
}
VulkanGraphicsEngine::~VulkanGraphicsEngine()
{
    delete m_runtimeBindings;

    delete m_preShadowFunc;
    delete m_postShadowFunc;
    delete m_preRenderFunc;
    delete m_postRenderFunc;
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
    if (m_freeShaderSlots.size() != m_shaderPrograms.Size())
    {
        Logger::Warning("Shader buffers out of sync. Likely leaked");
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
    const RenderProgram& program = m_shaderPrograms[a_pipeline];
    vk::RenderPass pass = m_swapchain->GetRenderPass();
    if (a_renderTexture != -1)
    {
        const VulkanRenderTexture* tex = m_renderTextures[a_renderTexture];
        pass = tex->GetRenderPass();
    }

    VulkanPipeline* pipeline = new VulkanPipeline(m_vulkanEngine, this, pass, program);
    {
        const std::lock_guard g = std::lock_guard(m_pipeLock);
        m_pipelines.emplace(addr, pipeline);
    }

    return pipeline;
}

vk::CommandBuffer VulkanGraphicsEngine::DrawCamera(uint32_t a_camIndex) 
{
    // While there is no code relating to mono in here for now.
    // This is used to fix a crash relating to locking a Thread after going from Mono -> Native 
    // When when another thread tries to aquire a lock after and it is not visible from Mono despite still being native will cause MemMap Crash 
    m_runtimeManager->AttachThread();
    
    const vk::Device device = m_vulkanEngine->GetLogicalDevice();
    const uint32_t curFrame = m_vulkanEngine->GetCurrentFrame();
    const RenderEngine* renderEngine = m_vulkanEngine->GetRenderEngine();
    ObjectManager* objectManager = renderEngine->GetObjectManager();

    const CameraBuffer& buffer = m_cameraBuffers[a_camIndex];

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

    VulkanRenderCommand& renderCommand = m_renderCommands.Push(VulkanRenderCommand(m_vulkanEngine, m_swapchain, commandBuffer));

    void* args[] = 
    { 
        &a_camIndex 
    };

    m_preRenderFunc->Exec(args);

    const std::vector<MaterialRenderStack> stacks = m_renderStacks.ToVector();

    for (const MaterialRenderStack& renderStack : stacks)
    {
        const uint32_t matAddr = renderStack.GetMaterialAddr();
        const RenderProgram& program = m_shaderPrograms[matAddr];
        if (buffer.RenderLayer & program.RenderLayer)
        {
            const glm::ivec2 renderSize = renderCommand.GetRenderSize();

            const glm::vec2 screenPos = buffer.View.Position * (glm::vec2)renderSize;
            const glm::vec2 screenSize = buffer.View.Size * (glm::vec2)renderSize;

            const vk::Rect2D scissor = vk::Rect2D({ (int32_t)screenPos.x, (int32_t)screenPos.y }, { (uint32_t)screenSize.x, (uint32_t)screenSize.y });
            commandBuffer.setScissor(0, 1, &scissor);

            const vk::Viewport viewport = vk::Viewport
            (
                screenPos.x,
                screenPos.y,
                screenSize.x,
                screenSize.y,
                buffer.View.MinDepth,
                buffer.View.MaxDepth
            );
            commandBuffer.setViewport(0, 1, &viewport);

            const VulkanPipeline* pipeline = GetPipeline(renderCommand.GetRenderTexutreAddr(), matAddr);

            pipeline->UpdateCameraBuffer(curFrame, screenSize, buffer, objectManager);
            pipeline->Bind(curFrame, commandBuffer);

            const std::vector<ModelBuffer> modelBuffers = renderStack.GetModelBuffers();
            for (const ModelBuffer& modelBuff : modelBuffers)
            {
                if (modelBuff.ModelAddr != -1)
                {
                    VulkanModel* model = m_models[modelBuff.ModelAddr];
                    const std::lock_guard mLock = std::lock_guard(model->GetLock());
                    if (model != nullptr)
                    {
                        model->Bind(commandBuffer);
                        const uint32_t indexCount = model->GetIndexCount();
                        for (uint32_t tAddr : modelBuff.TransformAddr)
                        {
                            TransformBuffer buffer = objectManager->GetTransformBuffer(tAddr);

                            pipeline->UpdateTransformBuffer(commandBuffer, curFrame, buffer, objectManager);

                            commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
                        }
                    }
                }
            }
        }
    }

    m_postRenderFunc->Exec(args);

    commandBuffer.endRenderPass();
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
    return m_vertexShaders[a_addr];
}
VulkanPixelShader* VulkanGraphicsEngine::GetPixelShader(uint32_t a_addr)
{
    return m_pixelShaders[a_addr];
}