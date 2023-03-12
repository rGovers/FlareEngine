using FlareEngine.Maths;
using System.Runtime.CompilerServices;

namespace FlareEngine
{
    public enum MouseButton : ushort
    {
        Left = 0,
        Middle = 1,
        Right = 2
    }

    public static class Input
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static Vector2 GetCursorPos();

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetMouseDownState(uint a_button);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetMousePressedState(uint a_button);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetMouseReleasedState(uint a_button);

        public delegate void MouseCallback(MouseButton a_button);

        public static MouseCallback PressedCallback;
        public static MouseCallback ReleasedCallback;

        public static Vector2 CursorPos
        {
            get
            {
                return GetCursorPos();
            }
        }

        public static bool IsMouseDown(MouseButton a_button)
        {
            return GetMouseDownState((uint)a_button) != 0;
        }
        public static bool IsMouseUp(MouseButton a_button)
        {
            return GetMouseDownState((uint)a_button) == 0;
        }
        public static bool IsMousePressed(MouseButton a_button)
        {
            return GetMousePressedState((uint)a_button) != 0;
        }
        public static bool IsMouseReleased(MouseButton a_button)
        {
            return GetMouseReleasedState((uint)a_button) != 0;
        }

        static void MousePressedEvent(uint a_button)
        {
            if (PressedCallback != null)
            {
                PressedCallback((MouseButton)a_button);
            }
        }
        static void MouseReleasedEvent(uint a_button)
        {
            if (ReleasedCallback != null)
            {
                ReleasedCallback((MouseButton)a_button);
            }
        }
    }
}