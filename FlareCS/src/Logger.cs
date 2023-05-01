using System.Runtime.CompilerServices;

namespace FlareEngine
{
    public static class Logger
    {
        public delegate void MessageStream(string a_msg);

        public static MessageStream MessageCallback = null;
        public static MessageStream WarningCallback = null;
        public static MessageStream ErrorCallback = null;

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void PushMessage(string a_message);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void PushWarning(string a_message);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void PushError(string a_message);

        internal static void FlareMessage(string a_message)
        {
            if (Application.IsEditor)
            {
                Message($"FlareCSE: {a_message}");
            }
            else
            {
                Message($"FlareCS: {a_message}");
            }
        }
        internal static void FlareWarning(string a_message)
        {
            if (Application.IsEditor)
            {
                Warning($"FlareCSE: {a_message}");
            }
            else
            {
                Warning($"FlareCS: {a_message}");
            }
        }
        internal static void FlareError(string a_message)
        {
            if (Application.IsEditor)
            {
                Error($"FlareCSE: {a_message}");
            }
            else
            {
                Error($"FlareCS: {a_message}");
            }
        }

        public static void Message(string a_message)
        {
            PushMessage(a_message);
            if (MessageCallback != null)
            {
                MessageCallback(a_message);
            }
        }
        public static void Warning(string a_message)
        {
            PushWarning(a_message);
            if (WarningCallback != null)
            {
                WarningCallback(a_message);
            }
        }
        public static void Error(string a_message)
        {
            PushError(a_message);
            if (ErrorCallback != null)
            {
                ErrorCallback(a_message);
            }
        }
    }
}