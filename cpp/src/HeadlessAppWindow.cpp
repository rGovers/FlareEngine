#include "AppWindow/HeadlessAppWindow.h"

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#ifndef WIN32
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#endif

#include <cassert>
#include <filesystem>
#include <string>

#include "Trace.h"

static std::string GetAddr(const std::string_view& a_addr)
{
    return std::filesystem::temp_directory_path().string() + std::string(a_addr);
}

void HeadlessAppWindow::MessageCallback(const std::string_view& a_message, e_LoggerMessageType a_type)
{
    const uint32_t strSize = (uint32_t)a_message.size();
    constexpr uint32_t TypeSize = sizeof(e_LoggerMessageType);
    const uint32_t size = strSize + TypeSize;

    PipeMessage msg;
    msg.Type = PipeMessageType_Message;
    msg.Length = size;
    msg.Data = new char[size];
    memcpy(msg.Data, &a_type, TypeSize);
    memcpy(msg.Data + TypeSize, a_message.data(), strSize);

    m_queuedMessages.Push(msg);
}

HeadlessAppWindow::HeadlessAppWindow()
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
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        Logger::Error("Failed to start WSA");
        assert(0);
    }

    m_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_sock != INVALID_SOCKET)
    {
        Logger::Error("Failed creating IPC");
        perror("socket");
        assert(0);
    }

    sockaddr_un addr;
    ZeroMemory(&addr, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy_s(addr.sun_path, addrStr.c_str());
    
    if (connect(m_sock, (sockaddr*)&addr, sizeof(addr)))
    {
        Logger::Error("FlareEngine: Failed to connect to IPC");
        perror("connect");
        assert(0);
    }
#else
    m_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    assert(m_sock >= 0);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, addrStr.c_str());
    
    if (connect(m_sock, (struct sockaddr*)&addr, SUN_LEN(&addr)) < 0)
    {
        Logger::Error("FlareEngine: Failed to connect to IPC");
        perror("connect");
        assert(0);
    }
#endif

    const PipeMessage msg = ReceiveMessage();
    const glm::ivec2 size = *(glm::ivec2*)msg.Data;

    m_width = (uint32_t)size.x;
    m_height = (uint32_t)size.y;
    delete[] msg.Data;

    Logger::CallbackFunc = new Logger::Callback(std::bind(&HeadlessAppWindow::MessageCallback, this, std::placeholders::_1, std::placeholders::_2));

    m_prevTime = std::chrono::high_resolution_clock::now();

    TRACE("Headless Window Initialised");
}
HeadlessAppWindow::~HeadlessAppWindow()
{
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
}

PipeMessage HeadlessAppWindow::ReceiveMessage() const
{
    PipeMessage msg;
#if WIN32
    const uint32_t size = (uint32_t)recv(m_sock, (char*)&msg, PipeMessage::Size, 0);
    if (size >= PipeMessage::Size)
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
    const uint32_t size = (uint32_t)read(m_sock, &msg, PipeMessage::Size);
    if (size >= PipeMessage::Size)
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
    
    return PipeMessage();
}
void HeadlessAppWindow::PushMessage(const PipeMessage& a_message) const
{
#if WIN32
    send(m_sock, (const char*)&a_message, PipeMessage::Size, 0);
    if (a_message.Data != nullptr)
    {
        send(m_sock, a_message.Data, (int)a_message.Length, 0);
    }
#else
    write(m_sock, &a_message, PipeMessage::Size);
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
    return 0.0;
}
double HeadlessAppWindow::GetTime() const
{
    return 0.0;
}

bool HeadlessAppWindow::PollMessage()
{
    bool ret = true;

    const PipeMessage msg = ReceiveMessage();

    switch (msg.Type)
    {
    case PipeMessageType_Close:
    {
        m_close = true;

        break;
    }
    case PipeMessageType_UnlockFrame:
    {
        m_unlockWindow = true;

        break;
    }
    case PipeMessageType_Resize:
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
    case PipeMessageType_Null:
    {
        ret = false;

        break;
    }
    default:
    {
        Logger::Error("Invalid Message: " + std::to_string(msg.Type));
        
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
#if WIN32
    if (m_sock != INVALID_SOCKET)
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
    if (m_sock >= 0)
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

    const std::chrono::time_point time = std::chrono::high_resolution_clock::now();

    m_delta = std::chrono::duration<double>(time - m_prevTime).count();
    m_time += m_delta;

    m_prevTime = time;

    const glm::dvec2 tVec = glm::vec2(m_delta, m_time);

    PushMessage({ PipeMessageType_FrameData, sizeof(glm::dvec2), (char*)&tVec});

    if (m_frameData != nullptr && m_unlockWindow)
    {
        m_unlockWindow = false;

        const std::lock_guard g = std::lock_guard(m_fLock);
        
        PushMessage({ PipeMessageType_PushFrame, m_width * m_height * 4, m_frameData });
    }

    if (!m_queuedMessages.Empty())
    {
        std::mutex& l = m_queuedMessages.Lock();

        l.lock();

        const uint32_t size = m_queuedMessages.Size();
        const PipeMessage* pipeMessages = m_queuedMessages.Data();

        for (uint32_t i = 0; i < size; ++i)
        {
            PushMessage(pipeMessages[i]);
            delete[] pipeMessages[i].Data;
        }

        l.unlock();

        m_queuedMessages.Clear();
    }
}

glm::ivec2 HeadlessAppWindow::GetSize() const
{
    return glm::ivec2((int)m_width, (int)m_height);
}

void HeadlessAppWindow::PushFrameData(uint32_t a_width, uint32_t a_height, const char* a_buffer)
{
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