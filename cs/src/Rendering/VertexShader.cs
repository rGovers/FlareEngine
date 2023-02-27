using System;
using System.IO;
using System.Runtime.CompilerServices;

namespace FlareEngine.Rendering
{
    public class VertexShader : IDisposable
    {
        bool          m_disposed = false;
        readonly uint m_internalAddr;

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateGLSLShader(string a_shader); 
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateFShader(string a_shader);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyShader(uint a_addr);

        internal uint InternalAddr
        {
            get
            {
                return m_internalAddr;
            }
        }

        VertexShader(uint a_addr)
        {   
            m_internalAddr = a_addr;
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        public static VertexShader LoadVertexShader(string a_path)
        {
            if (File.Exists(a_path))
            {
                string str = File.ReadAllText(a_path);
                if (!string.IsNullOrWhiteSpace(str))
                {
                    switch (Path.GetExtension(a_path).ToLower())
                    {
                    case ".fvert":
                    {
                        return new VertexShader(GenerateFShader(str));
                    }
                    }

                    return new VertexShader(GenerateGLSLShader(str));
                }
                else
                {
                    Logger.FlareError($"VertexShader Empty: {a_path}");
                }
            }
            else
            {
                Logger.FlareError($"VertexShader does not exist: {a_path}");
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
                    Logger.FlareWarning("VertexShader Failed to Dispose");
                }

                m_disposed = true;
            }
            else
            {
                Logger.FlareError("FlareCS: Multiple VertexShader Dispose");
            }
        }

        ~VertexShader()
        {
            Dispose(false);
        }
    }
}