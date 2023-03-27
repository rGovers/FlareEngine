using FlareEngine.Definitions;
using FlareEngine.Mod;
using FlareEngine.Rendering;
using FlareEngine.Rendering.UI;
using System.Collections.Generic;

namespace FlareEngine
{
    public static class AssetLibrary
    {
        static Dictionary<string, Material>     Materials;
        static Dictionary<string, VertexShader> VertexShaders;
        static Dictionary<string, PixelShader>  PixelShaders;
        static Dictionary<string, Font>         Fonts;

        internal static void Init()
        {
            Materials = new Dictionary<string, Material>();

            VertexShaders = new Dictionary<string, VertexShader>();
            PixelShaders = new Dictionary<string, PixelShader>();

            Fonts = new Dictionary<string, Font>();
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

            foreach (Font font in Fonts.Values)
            {
                font.Dispose();
            }
            Fonts.Clear();
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
        public static Font LoadFont(string a_path)
        {
            if (Fonts.ContainsKey(a_path))
            {
                return Fonts[a_path];
            }

            string filepath = ModControl.GetAssetPath(a_path);
            if (string.IsNullOrEmpty(filepath))
            {
                Logger.FlareError($"Cannot find filepath: {a_path}");

                return null;
            }

            Font font = Font.LoadFont(filepath);
            if (font == null)
            {
                Logger.FlareError($"Error loading Font: {a_path} at {filepath}");
            }

            Fonts.Add(a_path, font);

            return font;
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