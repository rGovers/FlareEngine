#include "InputManager.h"

#include "Runtime/RuntimeFunction.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

static InputManager* Instance = nullptr;

#define INPUTMANAGER_RUNTIME_ATTACH(ret, namespace, klass, name, code, ...) a_runtime->BindFunction(RUNTIME_FUNCTION_STRING(namespace, klass, name), (void*)RUNTIME_FUNCTION_NAME(klass, name));

#define INPUTMANAGER_BINDING_FUNCTION_TABLE(F) \
    F(glm::vec2, FlareEngine, Input, GetCursorPos, { return Instance->GetCursorPos(); }) \
    F(uint32_t, FlareEngine, Input, GetMouseDownState, { return (uint32_t)Instance->IsMouseDown((e_MouseButton)a_button); }, uint32_t a_button) \
    F(uint32_t, FlareEngine, Input, GetMousePressedState, { return (uint32_t)Instance->IsMousePressed((e_MouseButton)a_button); }, uint32_t a_button) \
    F(uint32_t, FlareEngine, Input, GetMouseReleasedState, { return (uint32_t)Instance->IsMouseReleased((e_MouseButton)a_button); }, uint32_t a_button)

INPUTMANAGER_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

InputManager::InputManager(RuntimeManager* a_runtime)
{
    Instance = this;

    TRACE("Initializing Input Manager");

    m_mouseButton = 0;
    m_curPos = glm::vec2(0.0f);

    INPUTMANAGER_BINDING_FUNCTION_TABLE(INPUTMANAGER_RUNTIME_ATTACH);

    m_mousePressedFunc = a_runtime->GetFunction("FlareEngine", "Input", ":MousePressedEvent(uint)");
    m_mouseReleasedFunc = a_runtime->GetFunction("FlareEngine", "Input", ":MouseReleasedEvent(uint)");
}
InputManager::~InputManager()
{
    delete m_mousePressedFunc;
    delete m_mouseReleasedFunc;
}

void InputManager::SetMouseButton(e_MouseButton a_button, bool a_state)
{
    const uint32_t buttonIndex = a_button * 2 + 0;
    const uint32_t prevButtonIndex = a_button * 2 + 1;

    const bool prevState = m_mouseButton & 0b1 << buttonIndex;
    if (a_state)
    {
        if (!(prevState))
        {
            uint32_t but = (uint32_t)a_button;
            void* args[] =
            {
                &but
            };

            m_mousePressedFunc->Exec(args);
        }

        m_mouseButton |= 0b1 << buttonIndex;
    }
    else
    {
        if (prevState)
        {
            uint32_t but = (uint32_t)a_button;
            void* args[] =
            {
                &but
            };

            m_mouseReleasedFunc->Exec(args);
        }

        m_mouseButton &= ~(0b1 << buttonIndex);
    }

    if (prevState)
    {
        m_mouseButton |= 0b1 << prevButtonIndex;
    }
    else
    {
        m_mouseButton &= ~(0b1 << prevButtonIndex);
    }
}

void InputManager::Update()
{
    // Required for frame events can get multiple press and release events cause majority of time engine runs faster then editor 
    for (uint32_t i = 0; i < MouseButton_Last; ++i)
    {
        if (m_mouseButton & 0b1 << (i * 2))
        {
            m_mouseButton |= 0b1 << (i * 2 + 1);
        }
        else
        {
            m_mouseButton &= ~(0b1 << (i * 2 + 1));
        }
    }
}