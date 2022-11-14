#include "Rendering/Vulkan/VulkanGraphicsEngine.h"

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
#include "Trace.h"

VulkanGraphicsEngine::VulkanGraphicsEngine(RuntimeManager* a_runtime, VulkanRenderEngineBackend* a_vulkanEngine)
{
    m_vulkanEngine = a_vulkanEngine;

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

    m_runtimeBindings = new VulkanGraphicsEngineBindings(a_runtime, this);
}
VulkanGraphicsEngine::~VulkanGraphicsEngine()
{
    delete m_runtimeBindings;

    const vk::Device device = m_vulkanEngine->GetLogicalDevice();

    TRACE("Deleting command pool");
    device.destroyCommandPool(m_commandPool);

    TRACE("Deleting Pipelines")
    for (const auto& iter : m_pipelines)
    {
        delete iter.second;
    }

    TRACE("Checking if shaders where deleted");
    for (const VulkanVertexShader* shader : m_vertexShaders)
    {
        if (shader != nullptr)
        {
            Logger::Warning("Vertex Shader was not destroyed");

            delete shader;
        }
    }

    for (const VulkanPixelShader* shader : m_pixelShaders)
    {
        if (shader != nullptr)
        {
            Logger::Warning("Pixel Shader was not destroyed");

            delete shader;
        }
    }

    TRACE("Checking if models where deleted");
    for (const VulkanModel* model : m_models)
    {
        if (model != nullptr)
        {
            Logger::Warning("Model was not destroyed");

            delete model;
        }
    }

    TRACE("Checking camera buffer health");
    if (m_cameraBuffers.size() != 0)
    {
        Logger::Warning("Leaked Camera Buffer");
    }

    TRACE("Checking shader program buffer health");
    if (m_freeShaderSlots.size() != m_shaderPrograms.size())
    {
        Logger::Warning("Shader buffers out of sync. Likely leaked");
    }
}

std::vector<vk::CommandBuffer> VulkanGraphicsEngine::Update(const VulkanSwapchain* a_swapchain)
{
    const RenderEngine* renderEngine = m_vulkanEngine->GetRenderEngine();
    const ObjectManager* objectManager = renderEngine->GetObjectManager();
    const uint32_t curFrame = m_vulkanEngine->GetCurrentFrame();

    // TODO: Rewrite down the line
    const uint32_t camBufferSize = (uint32_t)m_cameraBuffers.size();
    const uint32_t renderBufferSize = (uint32_t)m_shaderPrograms.size();
    for (uint32_t i = 0; i < camBufferSize; ++i)
    {
        const CameraBuffer& camBuffer = m_cameraBuffers[i];

        for (uint32_t j = 0; j < renderBufferSize; ++j)
        {
            const RenderProgram& program = m_shaderPrograms[j];

            if (camBuffer.RenderLayer & program.RenderLayer)
            {
                const uint64_t ind = i | j << 32;
                const auto iter = m_pipelines.find(ind);
                if (iter == m_pipelines.end())
                {
                    TRACE("Creating Vulkan Pipeline");
                    m_pipelines.emplace(ind, new VulkanPipeline(m_vulkanEngine, this, a_swapchain->GetRenderPass(), i, program));
                }
            }
        }
    }

    const vk::Device device = m_vulkanEngine->GetLogicalDevice();

    std::vector<vk::CommandBuffer> bufferVec;

    const vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo
    (
        m_commandPool,
        vk::CommandBufferLevel::ePrimary,
        1
    );

    vk::CommandBuffer commandBuffer;
    if (device.allocateCommandBuffers(&allocInfo, &commandBuffer) != vk::Result::eSuccess)
    {   
        Logger::Error("Failed to Allocate Command Buffers \n");

        assert(0);
    }

    constexpr vk::CommandBufferBeginInfo beginInfo = vk::CommandBufferBeginInfo
    (
        vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    );

    commandBuffer.begin(beginInfo);

    const glm::ivec2 swapSize = a_swapchain->GetSize();
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

    for (const MaterialRenderStack& renderStack : m_renderStacks)
    {
        const uint32_t matAddr = renderStack.GetMaterialAddr();
        const RenderProgram& program = m_shaderPrograms[matAddr];
        for (uint32_t i = 0; i < camBufferSize; ++i)
        {
            const CameraBuffer& camBuff = m_cameraBuffers[i];
            if (camBuff.RenderLayer & program.RenderLayer)
            {
                const glm::vec2 screenPos = camBuff.View.Position * (glm::vec2)swapSize;
                const glm::vec2 screenSize = camBuff.View.Size * (glm::vec2)swapSize;

                const vk::Rect2D scissor = vk::Rect2D({ screenPos.x, screenPos.y }, { screenSize.x, screenSize.y });
                commandBuffer.setScissor(0, 1, &scissor);
                const vk::Viewport viewport = vk::Viewport
                (
                    screenPos.x, 
                    screenPos.y, 
                    screenSize.x, 
                    screenSize.y,
                    camBuff.View.MinDepth,
                    camBuff.View.MaxDepth
                );
                commandBuffer.setViewport(0, 1, &viewport); 

                VulkanPipeline* pipeline = m_pipelines[i | matAddr << 32];
                pipeline->UpdateCameraBuffer(curFrame, screenSize, camBuff, objectManager);

                pipeline->Bind(curFrame, commandBuffer);

                const std::vector<ModelBuffer> modelBuffers = renderStack.GetModelBuffers();
                for (const ModelBuffer& modelBuff : modelBuffers)
                {
                    if (modelBuff.ModelAddr != -1)
                    {
                        const VulkanModel* model = m_models[modelBuff.ModelAddr];
                        model->Bind(commandBuffer);
                        const uint32_t indexCount = model->GetIndexCount();

                        commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);                        
                    }
                }
            }
        }
    }

    commandBuffer.endRenderPass();
    commandBuffer.end();
    
    bufferVec.emplace_back(commandBuffer);

    return bufferVec;
}

VulkanVertexShader* VulkanGraphicsEngine::GetVertexShader(uint32_t a_addr) const
{
    return m_vertexShaders[a_addr];
}
VulkanPixelShader* VulkanGraphicsEngine::GetPixelShader(uint32_t a_addr) const
{
    return m_pixelShaders[a_addr];
}