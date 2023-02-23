#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <queue>
#include <vector>

#include "DataTypes/TArray.h"

class RuntimeManager;

struct TransformBuffer;

class ObjectManager
{
private:
    std::queue<uint32_t>    m_freeTransforms;
    TArray<TransformBuffer> m_transformBuffer;

protected:

public:
    ObjectManager(RuntimeManager* a_runtime);
    ~ObjectManager();

    inline std::vector<TransformBuffer> ToVector() 
    {
        return m_transformBuffer.ToVector();
    }

    uint32_t CreateTransformBuffer();
    TransformBuffer GetTransformBuffer(uint32_t a_addr);
    void SetTransformBuffer(uint32_t a_addr, const TransformBuffer& a_buffer);
    void DestroyTransformBuffer(uint32_t a_addr);

    glm::mat4 GetGlobalMatrix(uint32_t a_addr);
};

struct TransformBuffer
{
    uint32_t  Parent = -1;

    glm::vec3 Translation;
    glm::quat Rotation;
    glm::vec3 Scale;

    glm::mat4 ToMat4() const
    {
        constexpr glm::mat4 Iden = glm::identity<glm::mat4>();

        const glm::mat4 translation = glm::translate(Iden, Translation);
        const glm::mat4 rotation = glm::toMat4(Rotation);
        const glm::mat4 scale = glm::scale(Iden, Scale);

        return translation * rotation * scale;
    }

    glm::mat4 ToGlobalMat4(ObjectManager* a_objectManager) const
    {
        if (Parent != -1)
        {
            const TransformBuffer buffer = a_objectManager->GetTransformBuffer(Parent);

            return buffer.ToGlobalMat4(a_objectManager) * ToMat4();
        }

        return ToMat4();
    }
};