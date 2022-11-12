#include "AppWindow/HeadlessAppWindow.h"

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <assert.h>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <string>
#include <unistd.h>

#include "Trace.h"

HeadlessAppWindow::HeadlessAppWindow()
{
    TRACE("Creating headless window");

    m_close = false;

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
        printf("FlareEngine: Failed to connect to IPC \n");
        perror("connect");

        assert(0);
    }

    const PipeMessage msg = RecieveMessage();
    const glm::ivec2 size = *(glm::ivec2*)msg.Data;

    m_width = (uint32_t)size.x;
    m_height = (uint32_t)size.y;
    delete[] msg.Data;

    TRACE("Headless Window Initialised");
}
HeadlessAppWindow::~HeadlessAppWindow()
{
    close(m_sock);
}

PipeMessage HeadlessAppWindow::RecieveMessage() const
{
    PipeMessage msg;

    const uint32_t size = (uint32_t)read(m_sock, &msg, PipeMessage::Size);
    if (size >= 8)
    {
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

bool HeadlessAppWindow::ShouldClose() const
{
    return m_close;
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

}