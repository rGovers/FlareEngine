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
     
        static Dictionary<string, Model>        Models;
     
        static Dictionary<string, Font>         Fonts;

        internal static void Init()
        {
            Materials = new Dictionary<string, Material>();

            VertexShaders = new Dictionary<string, VertexShader>();
            PixelShaders = new Dictionary<string, PixelShader>();

            Models = new Dictionary<string, Model>();

            Fonts = new Dictionary<string, Font>();
        }

        public static void ClearAssets()
        {
            foreach (VertexShader vShader in VertexShaders.Values)
            {
                if (!vShader.IsDisposed)
                {
                    vShader.Dispose();
                }
            }
            VertexShaders.Clear();

            foreach (PixelShader pShader in PixelShaders.Values)
            {
                if (!pShader.IsDisposed)
                {
                    pShader.Dispose();
                }
            }
            PixelShaders.Clear();

            foreach (Material mat in Materials.Values)
            {
                if (!mat.IsDisposed)
                {
                    mat.Dispose();
                }
            }
            Materials.Clear();

            foreach (Model model in Models.Values)
            {
                if (!model.IsDisposed)
                {
                    model.Dispose();
                }
            }
            Models.Clear();

            foreach (Font font in Fonts.Values)
            {
                if (!font.IsDisposed)
                {
                    font.Dispose();
                }
            }
            Fonts.Clear();
        }

        public static VertexShader LoadVertexShader(string a_path)
        {
            if (VertexShaders.ContainsKey(a_path))
            {
                VertexShader vShader = VertexShaders[a_path];
                if (!vShader.IsDisposed)
                {
                    return vShader;
                }

                VertexShaders.Remove(a_path);
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
                PixelShader pShader = PixelShaders[a_path];
                if (!pShader.IsDisposed)
                {
                    return pShader;
                }

                PixelShaders.Remove(a_path);
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
                Font f = Fonts[a_path];
                if (!f.IsDisposed)
                {
                    return f;
                }

                Fonts.Remove(a_path);
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

                return null;
            }

            Fonts.Add(a_path, font);

            return font;
        }

        public static Model LoadModel(string a_path)
        {
            if (Models.ContainsKey(a_path))
            {
                Model m = Models[a_path];
                if (!m.IsDisposed)
                {
                    return m;
                }

                Models.Remove(a_path);
            }

            string filepath = ModControl.GetAssetPath(a_path);
            if (string.IsNullOrEmpty(filepath))
            {
                Logger.FlareError($"Cannot find filepath: {a_path}");

                return null;
            }

            Model model = Model.LoadModel(filepath);
            if (model == null)
            {
                Logger.FlareError($"Error loading Model: {a_path} at {filepath}");

                return null;
            }

            Models.Add(a_path, model);

            return model;
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
                Material m = Materials[str];
                if (!m.IsDisposed)
                {
                    return m;
                }

                Materials.Remove(str);
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