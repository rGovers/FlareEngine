using System;
using System.IO;
using System.Runtime.CompilerServices;

namespace FlareEngine.Rendering
{
    public class PixelShader : IDisposable
    {
        bool m_disposed = false;
        uint m_internalAddr;

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static uint GenerateShader(string a_shader); 
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
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
            }

            return null;
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
                    Console.Error.WriteLine("FlareCS: PixelShader Failed to Dispose");
                }

                m_disposed = true;
            }
            else
            {
                Console.Error.WriteLine("FlareCS: Multiple PixelShader Dispose");
            }
        }

        ~PixelShader()
        {
            Dispose(false);
        }
    }
}