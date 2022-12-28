using FlareEngine.Definitions;
using FlareEngine.Rendering;
using System.Collections.Generic;

namespace FlareEngine
{
    public static class AssetLibrary
    {
        static Dictionary<string, Material>     m_materials;
        static Dictionary<string, VertexShader> m_vertexShaders;
        static Dictionary<string, PixelShader>  m_pixelShaders;

        internal static void Init()
        {
            m_materials = new Dictionary<string, Material>();

            m_vertexShaders = new Dictionary<string, VertexShader>();
            m_pixelShaders = new Dictionary<string, PixelShader>();
        }

        public static void ClearAssets()
        {
            foreach (VertexShader vShader in m_vertexShaders.Values)
            {
                vShader.Dispose();
            }
            m_vertexShaders.Clear();

            foreach (PixelShader pShader in m_pixelShaders.Values)
            {
                pShader.Dispose();
            }
            m_pixelShaders.Clear();

            foreach (Material mat in m_materials.Values)
            {
                mat.Dispose();
            }
            m_materials.Clear();
        }

        public static VertexShader LoadVertexShader(string a_path)
        {
            if (m_vertexShaders.ContainsKey(a_path))
            {
                return m_vertexShaders[a_path];
            }

            VertexShader shader = VertexShader.LoadVertexShader(a_path);
            if (shader == null)
            {
                Logger.Error("FlareCS: Error loading VertexShader: " + a_path);

                return null;
            }

            m_vertexShaders.Add(a_path, shader);

            return shader;
        }
        public static PixelShader LoadPixelShader(string a_path)
        {
            if (m_pixelShaders.ContainsKey(a_path))
            {
                return m_pixelShaders[a_path];
            }

            PixelShader shader = PixelShader.LoadPixelShader(a_path);
            if (shader == null)
            {
                Logger.Error("FlareCS: Error loading PixelShader: " + a_path);

                return null;
            }

            m_pixelShaders.Add(a_path, shader);

            return shader;
        }

        public static Material GetMaterial(MaterialDef a_def)
        {
            if (a_def == null)
            {
                Logger.Warning("FlareCS: Null MaterialDef");

                return null;
            }

            string str = string.Format("[{0}] [{1}]", a_def.VertexShaderPath, a_def.PixelShaderPath);
            if (m_materials.ContainsKey(str))
            {
                return m_materials[str];
            }

            Material mat = Material.FromDef(a_def);
            if (mat != null)
            {
                m_materials.Add(str, mat);
            }

            return mat;
        }
    }
}