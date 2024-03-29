using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace FlareEngine.Rendering.UI
{
    public class Font : IDestroy
    {
        static Dictionary<uint, Font> BufferLookup = new Dictionary<uint, Font>();

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateFont(string a_path);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint DestroyFont(uint a_addr);

        uint m_bufferAddr = uint.MaxValue;

        public bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue;
            }
        }

        internal uint BufferAddr
        {
            get
            {
                return m_bufferAddr;
            }
        }

        Font(uint a_bufferAddr)
        {
            m_bufferAddr = a_bufferAddr;

            BufferLookup.Add(m_bufferAddr, this);
        }

        public static Font LoadFont(string a_path)
        {
            return new Font(GenerateFont(a_path));
        }

        internal static Font GetFont(uint a_buffer)
        {
            if (BufferLookup.ContainsKey(a_buffer))
            {
                return BufferLookup[a_buffer];
            }

            return null;
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool a_disposing)
        {
            if(m_bufferAddr != uint.MaxValue)
            {
                if(a_disposing)
                {
                    DestroyFont(m_bufferAddr);

                    BufferLookup.Remove(m_bufferAddr);
                }
                else
                {
                    Logger.FlareWarning("Font Failed to Dispose");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.FlareError("Multiple Font Dispose");
            }
        }

        ~Font()
        {
            Dispose(false);
        }
    }
}