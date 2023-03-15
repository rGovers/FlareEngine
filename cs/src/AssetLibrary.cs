using FlareEngine.Definitions;
using FlareEngine.Mod;
using FlareEngine.Rendering;
using System.Collections.Generic;

namespace FlareEngine
{
    public static class AssetLibrary
    {
        static string                           WorkingDir;
        static Dictionary<string, Material>     Materials;
        static Dictionary<string, VertexShader> VertexShaders;
        static Dictionary<string, PixelShader>  PixelShaders;

        internal static void Init(string a_workingDir)
        {
            WorkingDir = a_workingDir;

            Materials = new Dictionary<string, Material>();

            VertexShaders = new Dictionary<string, VertexShader>();
            PixelShaders = new Dictionary<string, PixelShader>();
        }

        public static void ClearAssets()
        {
            foreach (VertexShader vShader in VertexShaders.Values)
            {
                vShader.Dispose();
            }
            VertexShaders.Clear();

            foreach (PixelShader pShader in PixelShaders.Values)
            {
                pShader.Dispose();
            }
            PixelShaders.Clear();

            foreach (Material mat in Materials.Values)
            {
                mat.Dispose();
            }
            Materials.Clear();
        }

        public static VertexShader LoadVertexShader(string a_path)
        {
            if (VertexShaders.ContainsKey(a_path))
            {
                return VertexShaders[a_path];
            }

            string filepath = ModControl.GetAssetPath(a_path);
            if (string.IsNullOrEmpty(filepath))
            {
                Logger.FlareError($"Cannot find filepath: {a_path}");

                return null;
            }

            VertexShader shader = VertexShader.LoadVertexShader(filepath);
            if (shader == null)
            {
                Logger.FlareError($"Error loading VertexShader: {a_path}, at {filepath}");

                return null;
            }

            VertexShaders.Add(a_path, shader);

            return shader;
        }
        public static PixelShader LoadPixelShader(string a_path)
        {
            if (PixelShaders.ContainsKey(a_path))
            {
                return PixelShaders[a_path];
            }

            string filepath = ModControl.GetAssetPath(a_path);
            if (string.IsNullOrEmpty(filepath))
            {
                Logger.FlareError($"Cannot find filepath: {a_path}");

                return null;
            }

            PixelShader shader = PixelShader.LoadPixelShader(filepath);
            if (shader == null)
            {
                Logger.FlareError($"Error loading PixelShader: {a_path} at {filepath}");

                return null;
            }

            PixelShaders.Add(a_path, shader);

            return shader;
        }

        public static Material GetMaterial(MaterialDef a_def)
        {
            if (a_def == null)
            {
                Logger.FlareWarning("Null MaterialDef");

                return null;
            }

            string str = $"[{a_def.VertexShaderPath}] [{a_def.PixelShaderPath}]";
            if (Materials.ContainsKey(str))
            {
                return Materials[str];
            }

            Material mat = Material.FromDef(a_def);
            if (mat != null)
            {
                Materials.Add(str, mat);
            }

            return mat;
        }
    }
}