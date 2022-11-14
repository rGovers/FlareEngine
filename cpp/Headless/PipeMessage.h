#pragma once

#include <cstdint>

enum e_PipeMessageType : uint32_t
{
    PipeMessageType_Null = 0,
    PipeMessageType_Close,
    PipeMessageType_Resize,
    PipeMessageType_PushFrame,
    PipeMessageType_Message,
    PipeMessageType_EndStream
};

struct PipeMessage
{
    e_PipeMessageType Type;
    uint32_t Length;
    char* Data;

    static constexpr uint32_t Size = sizeof(Type) + sizeof(Length);
};