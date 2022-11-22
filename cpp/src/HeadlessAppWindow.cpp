#include "AppWindow/HeadlessAppWindow.h"

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <assert.h>
#include <cstdlib>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string>
#include <unistd.h>

#include "Trace.h"

void HeadlessAppWindow::MessageCallback(const std::string_view& a_message, e_LoggerMessageType a_type)
{
    const uint32_t strSize = (uint32_t)a_message.size();
    constexpr uint32_t TypeSize = sizeof(e_LoggerMessageType);
    const uint32_t size = strSize + TypeSize;;

    PipeMessage msg;
    msg.Type = PipeMessageType_Message;
    msg.Length = size;
    msg.Data = new char[size];
    memcpy(msg.Data, &a_type, TypeSize);
    memcpy(msg.Data + TypeSize, a_message.begin(), strSize);

    m_queuedMessages.Push(msg);
}

HeadlessAppWindow::HeadlessAppWindow()
{
    TRACE("Creating headless window");

    m_close = false;

    m_frameData = nullptr;

    const char* tempDir = std::getenv("TMPDIR");
    if (tempDir == nullptr)
    {
        tempDir = std::getenv("TMP");
    }
    if (tempDir == nullptr)
    {
        tempDir = std::getenv("TEMP");
    }
    if (tempDir == nullptr)
    {
        tempDir = std::getenv("XDG_RUNTIME_DIR");
    }

    assert(tempDir != nullptr);

    TRACE("Initialising IPC");
    const std::string addrStr = std::string(tempDir) + "/FlareEngine-IPC";

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

    const PipeMessage msg = RecieveMessage();
    const glm::ivec2 size = *(glm::ivec2*)msg.Data;

    m_width = (uint32_t)size.x;
    m_height = (uint32_t)size.y;
    delete[] msg.Data;

    Logger::CallbackFunc = new Logger::Callback(std::bind(&HeadlessAppWindow::MessageCallback, this, std::placeholders::_1, std::placeholders::_2));

    TRACE("Headless Window Initialised");
}
HeadlessAppWindow::~HeadlessAppWindow()
{
    if (m_sock >= 0)
    {
        close(m_sock);
    }

    if (m_frameData != nullptr)
    {
        delete[] m_frameData;
        m_frameData = nullptr;
    }

    delete Logger::CallbackFunc;
    Logger::CallbackFunc = nullptr;
}

PipeMessage HeadlessAppWindow::RecieveMessage() const
{
    PipeMessage msg;

    const uint32_t size = (uint32_t)read(m_sock, &msg, PipeMessage::Size);
    if (size >= 8)
    {
        if (msg.Length <= 0)
        {
            msg.Data = nullptr;

            return msg;
        }

        msg.Data = new char[msg.Length];
        char* DataBuffer = msg.Data;
        uint32_t len = DataBuffer - msg.Data;
        while (len < msg.Length)
        {
            DataBuffer += read(m_sock, DataBuffer, msg.Length - len);

            len = DataBuffer - msg.Data;
        }

        return msg;
    }

    msg.Type = PipeMessageType_Null;
    msg.Length = 0;
    msg.Data = nullptr;

    return msg;
}
void HeadlessAppWindow::PushMessage(const PipeMessage& a_message) const
{
    write(m_sock, &a_message, PipeMessage::Size);
    if (a_message.Data != nullptr)
    {
        write(m_sock, a_message.Data, a_message.Length);
    }
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

void HeadlessAppWindow::Update()
{
    if (m_sock == -1)
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
            const PipeMessage msg = RecieveMessage();

            switch (msg.Type)
            {
            case PipeMessageType_Close:
            {
                m_close = true;

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
            }
            
            if (msg.Data)
            {
                delete[] msg.Data;
            }
        }
    }

    if (m_frameData != nullptr)
    {
        const std::lock_guard g = std::lock_guard(m_fLock);
        PipeMessage msg;
        msg.Type = PipeMessageType_PushFrame;
        msg.Length = m_width * m_height * 4;
        msg.Data = m_frameData;

        PushMessage(msg);
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