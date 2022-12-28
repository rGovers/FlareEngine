using System;
using System.IO;
using System.Runtime.CompilerServices;

namespace FlareEngine.Rendering
{
    public class PixelShader : IDisposable
    {
        bool          m_disposed = false;
        readonly uint m_internalAddr;

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateShader(string a_shader); 
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyShader(uint a_addr);

        public PixelShader(string a_shader)
        {
            m_internalAddr = GenerateShader(a_shader);
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        public static PixelShader LoadPixelShader(string a_path)
        {
            if (File.Exists(a_path))
            {
                string str = File.ReadAllText(a_path);
                if (!string.IsNullOrWhiteSpace(str))
                {
                    return new PixelShader(str);
                }
                else
                {
                    Logger.Error($"FlareCS: PixelShader Empty: {a_path}");
                }
            }
            else
            {
                Logger.Error($"FlareCS: PixelShader does not exist: {a_path}");
            }

            return null;
        }

        internal uint InternalAddr
        {
            get
            {
                return m_internalAddr;
            }
        }

        protected virtual void Dispose(bool a_disposing)
        {
            if(!m_disposed)
            {
                if(a_disposing)
                {
                    DestroyShader(m_internalAddr);
                }
                else
                {
                    Logger.Error("FlareCS: PixelShader Failed to Dispose");
                }

                m_disposed = true;
            }
            else
            {
                Logger.Error("FlareCS: Multiple PixelShader Dispose");
            }
        }

        ~PixelShader()
        {
            Dispose(false);
        }
    }
}