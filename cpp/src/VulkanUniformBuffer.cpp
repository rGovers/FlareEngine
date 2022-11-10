#include "Rendering/Vulkan/VulkanUniformBuffer.h"

#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

VulkanUniformBuffer::VulkanUniformBuffer(VulkanRenderEngineBackend* a_engine, uint32_t a_uniformSize)
{
    TRACE("Creating Vulkan UBO");
    m_engine = a_engine;

    m_bufferCount = VulkanRenderEngineBackend::MaxFlightFrames;
    m_uniformSize = a_uniformSize;

    m_buffers = new vk::Buffer[m_bufferCount];
    m_allocations = new VmaAllocation[m_bufferCount];

    for (uint32_t i = 0; i < m_bufferCount; ++i)
    {
        const VmaAllocator allocator = m_engine->GetAllocator();

        VkBufferCreateInfo bufferInfo = { };
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = (VkDeviceSize)a_uniformSize;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo bufferAllocInfo = { 0 };
        bufferAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        bufferAllocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        bufferAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        VkBuffer tBuffer;
        if (vmaCreateBuffer(allocator, &bufferInfo, &bufferAllocInfo, &tBuffer, &m_allocations[i], nullptr) != VK_SUCCESS)
        {
            printf("Failed to create uniform buffer \n");

            assert(0);
        }
        m_buffers[i] = tBuffer;
    }
}
VulkanUniformBuffer::~VulkanUniformBuffer()
{
    TRACE("Destroying UBO");
    const VmaAllocator allocator = m_engine->GetAllocator();
    
    for (uint32_t i = 0; i < m_bufferCount; ++i)
    {
        vmaDestroyBuffer(allocator, m_buffers[i], m_allocations[i]);
    }

    delete[] m_buffers;
    delete[] m_allocations;
}

void VulkanUniformBuffer::SetData(uint32_t a_index, const char* a_data)
{
    const VmaAllocator allocator = m_engine->GetAllocator();

    void* dat;
    vmaMapMemory(allocator, m_allocations[a_index], &dat);

    memcpy(dat, a_data, m_uniformSize);

    vmaUnmapMemory(allocator, m_allocations[a_index]);
}