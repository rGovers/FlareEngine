#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include "InputBindings.h"

class RuntimeFunction;
class RuntimeManager;

class InputManager
{
private:
    RuntimeFunction* m_mousePressedFunc;
    RuntimeFunction* m_mouseReleasedFunc;
    RuntimeFunction* m_keyPressedFunc;
    RuntimeFunction* m_keyReleasedFunc;

    glm::vec2        m_curPos;
 
    unsigned char    m_mouseButton;

    KeyboardState    m_curKeyState;
    KeyboardState    m_prevKeyState;

protected:

public:
    InputManager(RuntimeManager* a_runtime);
    ~InputManager();

    inline void SetCursorPos(const glm::vec2& a_pos)
    {
        m_curPos = a_pos;
    }
    inline glm::vec2 GetCursorPos() const
    {
        return m_curPos;
    }

    void SetMouseButton(e_MouseButton a_button, bool a_state);
    inline bool IsMouseDown(e_MouseButton a_button) const
    {
        return m_mouseButton & 0b1 << (a_button * 2);
    }
    inline bool IsMousePressed(e_MouseButton a_button) const
    {
        return m_mouseButton & 0b1 << (a_button * 2 + 0) && !(m_mouseButton & 0b1 << (a_button * 2 + 1));
    }
    inline bool IsMouseReleased(e_MouseButton a_button) const
    {
        return !(m_mouseButton & 0b1 << (a_button * 2 + 0)) && m_mouseButton & 0b1 << (a_button * 2 + 1);
    }

    void SetKeyboardKey(e_KeyCode a_keyCode, bool a_state);
    inline bool IsKeyDown(e_KeyCode a_keyCode) const
    {
        return m_curKeyState.IsKeyDown(a_keyCode);
    }
    inline bool IsKeyPressed(e_KeyCode a_keyCode) const
    {
        return m_curKeyState.IsKeyDown(a_keyCode) && !m_prevKeyState.IsKeyDown(a_keyCode);
    }
    inline bool IsKeyReleased(e_KeyCode a_keyCode) const
    {
        return !m_curKeyState.IsKeyDown(a_keyCode) && m_prevKeyState.IsKeyDown(a_keyCode);
    }

    void Update();
};