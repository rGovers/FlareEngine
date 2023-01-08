#pragma once

#include <cstdint>

enum e_PipeMessageType : uint32_t
{
    PipeMessageType_Null = 0,
    PipeMessageType_Close,
    PipeMessageType_Resize,
    PipeMessageType_FrameData,
    PipeMessageType_UpdateData,
    PipeMessageType_UnlockFrame,
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

    PipeMessage()
    {
        Type = PipeMessageType_Null;
        Length = 0;
        Data = nullptr;
    }
    PipeMessage(e_PipeMessageType a_type)
    {
        Type = a_type;
        Length = 0;
        Data = nullptr;
    }
    PipeMessage(e_PipeMessageType a_type, uint32_t a_dataLength, char* a_data)
    {
        Type = a_type;
        Length = a_dataLength;
        Data = a_data;
    }
};