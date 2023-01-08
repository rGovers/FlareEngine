#include "ObjectManager.h"

#include "Runtime/RuntimeManager.h"
#include "Trace.h"

static ObjectManager* OManager = nullptr;

FLARE_MONO_EXPORT(uint32_t, Transform_GenerateTransformBuffer)
{
    return OManager->CreateTransformBuffer();
}
FLARE_MONO_EXPORT(TransformBuffer, Transform_GetTransformBuffer, uint32_t a_addr)
{
    return OManager->GetTransformBuffer(a_addr);
}
FLARE_MONO_EXPORT(void, Transform_SetTransformBuffer, uint32_t a_addr, TransformBuffer a_buffer)
{
    OManager->SetTransformBuffer(a_addr, a_buffer);
}
FLARE_MONO_EXPORT(void, Transform_DestroyTransformBuffer, uint32_t a_addr)
{
    OManager->DestroyTransformBuffer(a_addr);
}

ObjectManager::ObjectManager(RuntimeManager* a_runtime)
{
    OManager = this;

    TRACE("Binding Object functions to C#");
    a_runtime->BindFunction("FlareEngine.Transform::GenerateTransformBuffer", (void*)Transform_GenerateTransformBuffer);
    a_runtime->BindFunction("FlareEngine.Transform::GetTransformBuffer", (void*)Transform_GetTransformBuffer);
    a_runtime->BindFunction("FlareEngine.Transform::SetTransformBuffer", (void*)Transform_SetTransformBuffer);
    a_runtime->BindFunction("FlareEngine.Transform::DestroyTransformBuffer", (void*)Transform_DestroyTransformBuffer);
}
ObjectManager::~ObjectManager()
{

}

uint32_t ObjectManager::CreateTransformBuffer()
{
    TransformBuffer buffer;
    buffer.Parent = -1;

    TRACE("Creating Transform Buffer");
    if (!m_freeTransforms.empty())
    {
        const uint32_t add = m_freeTransforms.front();
        m_freeTransforms.pop();

        m_transformBuffer[add] = buffer;

        return add;
    }

    TRACE("Allocating Transform Buffer");

    m_transformBuffer.Push(buffer);

    return m_transformBuffer.Size() - 1;
}
TransformBuffer ObjectManager::GetTransformBuffer(uint32_t a_addr)
{
    return m_transformBuffer[a_addr];
}
void ObjectManager::SetTransformBuffer(uint32_t a_addr, const TransformBuffer& a_buffer)
{
    m_transformBuffer[a_addr] = a_buffer;
}
void ObjectManager::DestroyTransformBuffer(uint32_t a_addr)
{
    TRACE("Destroying Transform Buffer");

    m_freeTransforms.emplace(a_addr);
}