#include "Rendering/Vulkan/VulkanGraphicsEngine.h"

#include <future>
#include <mutex>

#include "Logger.h"
#include "ObjectManager.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/ShaderBuffers.h"
#include "Rendering/Vulkan/VulkanGraphicsEngineBindings.h"
#include "Rendering/Vulkan/VulkanModel.h"
#include "Rendering/Vulkan/VulkanPipeline.h"
#include "Rendering/Vulkan/VulkanPixelShader.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
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

    m_preShadowFunc = m_runtimeManager->GetFunction("FlareEngine.Rendering", "RenderPipeline", ":PreShadow(uint)");
    m_postShadowFunc = m_runtimeManager->GetFunction("FlareEngine.Rendering", "RenderPipeline", ":PostShadow(uint)");
    m_preRenderFunc = m_runtimeManager->GetFunction("FlareEngine.Rendering", "RenderPipeline", ":PreRender(uint)");
    m_postRenderFunc = m_runtimeManager->GetFunction("FlareEngine.Rendering", "RenderPipeline", ":PostRender(uint)");
    m_postProcessFunc = m_runtimeManager->GetFunction("FlareEngine.Rendering", "RenderPipeline", ":PostProcess(uint)"); 
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
}

vk::CommandBuffer VulkanGraphicsEngine::DrawCamera(uint32_t a_camIndex, const VulkanSwapchain* a_swapchain) 
{
    // While there is no code relating to mono in here for now.
    // This is used to fix a crash relating to locking a Thread after going from Mono -> Native 
    // When when another thread tries to aquire a lock after and it is not visible from Mono despite still being native will cause MemMap Crash 
    m_runtimeManager->AttachThread();
    
    const vk::Device device = m_vulkanEngine->GetLogicalDevice();
    const uint32_t curFrame = m_vulkanEngine->GetCurrentFrame();
    const RenderEngine* renderEngine = m_vulkanEngine->GetRenderEngine();
    ObjectManager* objectManager = renderEngine->GetObjectManager();
    const glm::ivec2 swapSize = a_swapchain->GetSize();

    CameraBuffer& buffer = m_cameraBuffers[a_camIndex];

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

    constexpr vk::ClearValue ClearColor = vk::ClearValue(vk::ClearColorValue(std::array{ 0.1f, 0.1f, 0.1f, 1.0f }));

    const vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo
    (
        a_swapchain->GetRenderPass(),
        a_swapchain->GetFramebuffer(m_vulkanEngine->GetImageIndex()),
        vk::Rect2D({ 0, 0 }, { (uint32_t)swapSize.x, (uint32_t)swapSize.y }),
        1,
        &ClearColor
    );
    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

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
            const glm::vec2 screenPos = buffer.View.Position * (glm::vec2)swapSize;
            const glm::vec2 screenSize = buffer.View.Size * (glm::vec2)swapSize;

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

            // VulkanPipeline* pipeline = m_pipelines[a_camIndex | (uint64_t)matAddr << 32];
            const VulkanPipeline* pipeline = m_pipelines.at(a_camIndex | (uint64_t)matAddr << 32);

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

std::vector<vk::CommandBuffer> VulkanGraphicsEngine::Update(const VulkanSwapchain* a_swapchain)
{
    // To fix crash relating to thread locking and Mono
    m_runtimeManager->AttachThread();

    const vk::Device device = m_vulkanEngine->GetLogicalDevice();

    // TODO: Rewrite down the line
    const uint32_t camBufferSize = m_cameraBuffers.Size();
    const uint32_t renderBufferSize = m_shaderPrograms.Size();
    for (uint32_t i = 0; i < camBufferSize; ++i)
    {
        const CameraBuffer& camBuffer = m_cameraBuffers[i];

        for (uint32_t j = 0; j < renderBufferSize; ++j)
        {
            const RenderProgram& program = m_shaderPrograms[j];

            if (camBuffer.RenderLayer & program.RenderLayer)
            {
                const uint64_t ind = i | (uint64_t)j << 32;
                const auto iter = m_pipelines.find(ind);
                if (iter == m_pipelines.end())
                {
                    TRACE("Creating Vulkan Pipeline");
                    m_pipelines.emplace(ind, new VulkanPipeline(m_vulkanEngine, this, a_swapchain->GetRenderPass(), i, program));
                }
            }
        }
    }

    std::vector<std::future<vk::CommandBuffer>> futures;
    for (uint32_t i = 0; i < camBufferSize; ++i)
    {
        futures.emplace_back(std::async(std::bind(&VulkanGraphicsEngine::DrawCamera, this, i, a_swapchain)));
    }

    std::vector<vk::CommandBuffer> cmdBuffers;
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