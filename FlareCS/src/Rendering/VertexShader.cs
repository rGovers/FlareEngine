using System;
using System.IO;
using System.Runtime.CompilerServices;

namespace FlareEngine.Rendering
{
    public class VertexShader : IDestroy
    {
        uint m_internalAddr = uint.MaxValue;

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateGLSLShader(string a_shader); 
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateFShader(string a_shader);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyShader(uint a_addr);

        public bool IsDisposed
        {
            get
            {
                return m_internalAddr == uint.MaxValue;
            }
        }

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

        public override bool Equals(object a_obj)
        {
            if (a_obj == null && m_internalAddr == uint.MaxValue)
            {
                return true;
            }

            return base.Equals(a_obj);
        }
        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool a_disposing)
        {
            if(m_internalAddr != uint.MaxValue)
            {
                if(a_disposing)
                {
                    DestroyShader(m_internalAddr);
                }
                else
                {
                    Logger.FlareWarning("VertexShader Failed to Dispose");
                }

                m_internalAddr = uint.MaxValue;
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