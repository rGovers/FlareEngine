#include "AppWindow/HeadlessAppWindow.h"

#ifdef min
#undef min
#endif // min

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#ifndef WIN32
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#endif

#include <filesystem>
#include <string>

#include "Application.h"
#include "Flare/FlareAssert.h"
#include "InputManager.h"
#include "Profiler.h"
#include "Trace.h"

static std::string GetAddr(const std::string_view& a_addr)
{
    return (std::filesystem::temp_directory_path() / a_addr).string();
}

void HeadlessAppWindow::MessageCallback(const std::string_view& a_message, e_LoggerMessageType a_type)
{
    const uint32_t strSize = (uint32_t)a_message.size();
    constexpr uint32_t TypeSize = sizeof(e_LoggerMessageType);
    const uint32_t size = strSize + TypeSize;

    FlareBase::PipeMessage msg;
    msg.Type = FlareBase::PipeMessageType_Message;
    msg.Length = size;
    msg.Data = new char[size];
    memcpy(msg.Data, &a_type, TypeSize);
    memcpy(msg.Data + TypeSize, a_message.data(), strSize);

    m_queuedMessages.Push(msg);
}
void HeadlessAppWindow::ProfilerCallback(const Profiler::PData& a_profilerData)
{
    constexpr uint32_t ScopeSize = sizeof(ProfileScope);

    FlareBase::PipeMessage msg;
    msg.Type = FlareBase::PipeMessageType_ProfileScope;
    msg.Length = ScopeSize;
    msg.Data = new char[ScopeSize];

    ProfileScope* scope = (ProfileScope*)msg.Data;
    
    const int nameSize = glm::min((int)a_profilerData.Name.size(), NameMax - 1);
    for (int i = 0; i < nameSize; ++i)
    {
        scope->Name[i] = a_profilerData.Name[i];
    }
    scope->Name[nameSize] = 0;

    scope->FrameCount = (uint16_t)glm::min((int)a_profilerData.Frames.size(), FrameMax);
    for (uint16_t i = 0; i < scope->FrameCount; ++i)
    {
        const ProfileFrame& pFrame = a_profilerData.Frames[i];
        ProfileTFrame& frame = scope->Frames[i];

        const int frameNameSize = glm::min((int)pFrame.Name.size(), NameMax - 1);
        for (int j = 0; j < frameNameSize; ++j)
        {
            frame.Name[j] = pFrame.Name[j];
        }
        frame.Name[frameNameSize] = 0;
        frame.Stack = pFrame.Stack;
        frame.Time = std::chrono::duration<float>(pFrame.EndTime - pFrame.StartTime).count();
    }

    m_queuedMessages.Push(msg);
}

HeadlessAppWindow::HeadlessAppWindow(Application* a_app) : AppWindow(a_app)
{
    TRACE("Creating headless window");

    m_close = false;

    m_frameData = nullptr;
    m_unlockWindow = false;
    
    m_delta = 0.0;
    m_time = 0.0;

    TRACE("Initialising IPC");

    const std::string addrStr = GetAddr(PipeName);

#if WIN32
    WSADATA wsaData = { };
    FLARE_ASSERT_MSG_R(WSAStartup(MAKEWORD(2, 2), &wsaData) == 0, "Failed to start WSA");

    m_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_sock == INVALID_SOCKET)
    {
        Logger::Error("Failed creating IPC");
        perror("socket");
        FLARE_ASSERT(0);
    }

    sockaddr_un addr;
    ZeroMemory(&addr, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy_s(addr.sun_path, addrStr.c_str());
    
    if (connect(m_sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        Logger::Error("FlareEngine: Failed to connect to IPC");
        perror("connect");
        FLARE_ASSERT(0);
    }
#else
    m_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    FLARE_ASSERT_R(m_sock >= 0);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, addrStr.c_str());
    
    if (connect(m_sock, (struct sockaddr*)&addr, SUN_LEN(&addr)) < 0)
    {
        Logger::Error("FlareEngine: Failed to connect to IPC");
        perror("connect");
        FLARE_ASSERT(0);
    }
#endif

    const FlareBase::PipeMessage msg = ReceiveMessage();
    const glm::ivec2 size = *(glm::ivec2*)msg.Data;

    m_width = (uint32_t)size.x;
    m_height = (uint32_t)size.y;
    delete[] msg.Data;

    Logger::CallbackFunc = new Logger::Callback(std::bind(&HeadlessAppWindow::MessageCallback, this, std::placeholders::_1, std::placeholders::_2));
    Profiler::CallbackFunc = new Profiler::Callback(std::bind(&HeadlessAppWindow::ProfilerCallback, this, std::placeholders::_1));

    m_prevTime = std::chrono::high_resolution_clock::now();

    TRACE("Headless Window Initialised");
}
HeadlessAppWindow::~HeadlessAppWindow()
{
    TRACE("Cleaning up Headless Window");

    PushMessageQueue();

    PushMessage({ FlareBase::PipeMessageType_Close });

#if WIN32
    if (m_sock != INVALID_SOCKET)
    {
        closesocket(m_sock);
    }

    WSACleanup();
#else
    if (m_sock >= 0)
    {
        close(m_sock);
    }
#endif

    if (m_frameData != nullptr)
    {
        delete[] m_frameData;
        m_frameData = nullptr;
    }

    delete Logger::CallbackFunc;
    Logger::CallbackFunc = nullptr;
    delete Profiler::CallbackFunc;
    Profiler::CallbackFunc = nullptr;
}

void HeadlessAppWindow::PushMessageQueue()
{
    if (!m_queuedMessages.Empty())
    {
        TLockArray<FlareBase::PipeMessage> a = m_queuedMessages.ToLockArray();

        const uint32_t size = a.Size();

        for (uint32_t i = 0; i < size; ++i)
        {
            PushMessage(a[i]);   

            delete[] a[i].Data;
        }

        m_queuedMessages.UClear();
    }
}

FlareBase::PipeMessage HeadlessAppWindow::ReceiveMessage() const
{
    FlareBase::PipeMessage msg;
#if WIN32
    const int size = recv(m_sock, (char*)&msg, FlareBase::PipeMessage::Size, 0);
    if (size >= FlareBase::PipeMessage::Size)
    {
        if (msg.Length <= 0)
        {
            msg.Data = nullptr;

            return msg;
        }

        msg.Data = new char[msg.Length];
        char* dataBuffer = msg.Data;
        uint32_t len = (uint32_t)(dataBuffer - msg.Data);
        while (len < msg.Length)
        {
            dataBuffer += recv(m_sock, dataBuffer, (int)(msg.Length - len), 0);

            len = (uint32_t)(dataBuffer - msg.Data);
        }

        return msg;
    }
#else
    const uint32_t size = (uint32_t)read(m_sock, &msg, FlareBase::PipeMessage::Size);
    if (size >= FlareBase::PipeMessage::Size)
    {
        if (msg.Length <= 0)
        {
            msg.Data = nullptr;

            return msg;
        }

        msg.Data = new char[msg.Length];
        char* dataBuffer = msg.Data;
        uint32_t len = (uint32_t)(dataBuffer - msg.Data);
        while (len < msg.Length)
        {
            dataBuffer += read(m_sock, dataBuffer, msg.Length - len);

            len = (uint32_t)(dataBuffer - msg.Data);
        }

        return msg;
    }
#endif
    
    return FlareBase::PipeMessage();
}
void HeadlessAppWindow::PushMessage(const FlareBase::PipeMessage& a_message) const
{
    FLARE_ASSERT(a_message.Type != FlareBase::PipeMessageType_Null);

#if WIN32
    // TODO: CRITICAL: Find a better way of handling messages seems to be 2-3 orders of magnitude slower on Windows
    send(m_sock, (const char*)&a_message, FlareBase::PipeMessage::Size, 0);
    if (a_message.Data != nullptr)
    {
        send(m_sock, a_message.Data, (int)a_message.Length, 0);
    }
#else
    write(m_sock, &a_message, FlareBase::PipeMessage::Size);
    if (a_message.Data != nullptr)
    {
        write(m_sock, a_message.Data, a_message.Length);
    }
#endif
}

bool HeadlessAppWindow::ShouldClose() const
{
    return m_close || m_sock < 0;
}

double HeadlessAppWindow::GetDelta() const
{
    return m_delta;
}
double HeadlessAppWindow::GetTime() const
{
    return m_time;
}

bool HeadlessAppWindow::PollMessage()
{
    bool ret = true;

    const FlareBase::PipeMessage msg = ReceiveMessage();

    switch (msg.Type)
    {
    case FlareBase::PipeMessageType_Close:
    {
        m_close = true;

        break;
    }
    case FlareBase::PipeMessageType_UnlockFrame:
    {
        m_unlockWindow = true;

        break;
    }
    case FlareBase::PipeMessageType_Resize:
    {
        const std::lock_guard g = std::lock_guard(m_fLock);
        const glm::ivec2 size = *(glm::ivec2*)msg.Data;

        m_width = (uint32_t)size.x;
        m_height = (uint32_t)size.y;

        if (m_frameData != nullptr)
        {
            delete[] m_frameData;
            m_frameData = nullptr;
        }

        break;
    }
    case FlareBase::PipeMessageType_CursorPos:
    {
        const Application* app = GetApplication();
        
        InputManager* inputManager = app->GetInputManager();
        
        inputManager->SetCursorPos(*(glm::vec2*)msg.Data);

        break;
    }
    case FlareBase::PipeMessageType_MouseState:
    {
        const Application* app = GetApplication();

        InputManager* inputManager = app->GetInputManager();
        
        const unsigned char mouseState = *(unsigned char*)msg.Data;

        inputManager->SetMouseButton(FlareBase::MouseButton_Left, mouseState & 0b1 << FlareBase::MouseButton_Left);
        inputManager->SetMouseButton(FlareBase::MouseButton_Middle, mouseState & 0b1 << FlareBase::MouseButton_Middle);
        inputManager->SetMouseButton(FlareBase::MouseButton_Right, mouseState & 0b1 << FlareBase::MouseButton_Right);

        break;
    }
    case FlareBase::PipeMessageType_KeyboardState:
    {
        const Application* app = GetApplication();

        InputManager* inputManager = app->GetInputManager();

        const FlareBase::KeyboardState state = FlareBase::KeyboardState::FromData((unsigned char*)msg.Data);

        for (unsigned int i = 0; i < FlareBase::KeyCode_Last; ++i)
        {
            const FlareBase::e_KeyCode keyCode = (FlareBase::e_KeyCode)i;

            inputManager->SetKeyboardKey(keyCode, state.IsKeyDown(keyCode));
        }

        break;
    }
    case FlareBase::PipeMessageType_Null:
    {
        Logger::Warning("Engine: Null Message");

        ret = false;

        break;
    }
    default:
    {
        Logger::Error("FlareEngine: Invalid Pipe Message: " + std::to_string(msg.Type) + " " + std::to_string(msg.Length));
        
        break;
    }
    }

    if (msg.Data)
    {
        delete[] msg.Data;
    }

    return ret;
}

void HeadlessAppWindow::Update()
{
    Profiler::StartFrame("Polling");
#if WIN32
    if (m_sock == INVALID_SOCKET)
    {
        return;
    }

    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 5;

    fd_set fdSet;
    FD_ZERO(&fdSet);
    FD_SET(m_sock, &fdSet);
    if (select((int)(m_sock + 1), &fdSet, NULL, NULL, &tv) > 0)
    {
        PollMessage();
    }
#else
    if (m_sock < 0)
    {
        return;
    }

    struct pollfd fds;
    fds.fd = m_sock;
    fds.events = POLLIN;

    while (poll(&fds, 1, 1) > 0)
    {
        if (fds.revents & (POLLNVAL | POLLERR | POLLHUP))
        {
            m_sock = -1;

            return;
        }

        if (fds.revents & POLLIN)
        {
            PollMessage();
        }
    }
#endif
    Profiler::StopFrame();

    {
        PROFILESTACK("Timing");
        const std::chrono::time_point time = std::chrono::high_resolution_clock::now();

        m_delta = std::chrono::duration<double>(time - m_prevTime).count();
        m_time += m_delta;

        m_prevTime = time;

        const glm::dvec2 tVec = glm::vec2(m_delta, m_time);

        PushMessage({ FlareBase::PipeMessageType_UpdateData, sizeof(glm::dvec2), (char*)&tVec});
    }

    {
        PROFILESTACK("Frame Data");
        if (m_frameData != nullptr && m_unlockWindow)
        {
            m_unlockWindow = false;

            const std::lock_guard g = std::lock_guard(m_fLock);

            PushMessage({ FlareBase::PipeMessageType_PushFrame, m_width * m_height * 4, m_frameData });
        }
    }

    {
        PROFILESTACK("Messages");
        PushMessageQueue();
    }
}

glm::ivec2 HeadlessAppWindow::GetSize() const
{
    return glm::ivec2((int)m_width, (int)m_height);
}

void HeadlessAppWindow::PushFrameData(uint32_t a_width, uint32_t a_height, const char* a_buffer, double a_delta, double a_time)
{
    PROFILESTACK("Frame Data");
    constexpr int Size = sizeof(glm::dvec2);

    FlareBase::PipeMessage msg;
    msg.Type = FlareBase::PipeMessageType_FrameData;
    msg.Length = Size;
    msg.Data = new char[Size];
    (*(glm::dvec2*)msg.Data).x = a_delta;
    (*(glm::dvec2*)msg.Data).y = a_time;
    m_queuedMessages.Push(msg);

    // TODO: Implement a better way of doing this 
    // Can end up ~32MiB which cannot keep up with a copy
    // Assuming I can do maths ~6GiB/s so yeah not upto par
    // Probably end up with a syncronised direct push at some point
    const std::lock_guard g = std::lock_guard(m_fLock);
    if (m_width == a_width && m_height == a_height)
    {
        const uint32_t size = m_width * m_height * 4;

        if (m_frameData == nullptr)
        {
            m_frameData = new char[size];
        }

        memcpy(m_frameData, a_buffer, size);
    }
}