using FlareEngine.Rendering.Lighting;
using System;

namespace FlareEngine.Rendering
{
    public abstract class RenderPipeline
    {
        static RenderPipeline Instance = null;

        public abstract void PreShadow(Camera a_camera);
        public abstract void PostShadow(Camera a_camera);

        public abstract void PreRender(Camera a_camera);
        public abstract void PostRender(Camera a_camera);

        public abstract Material PreLight(LightType a_lightType, Camera a_camera);
        public abstract void PostLight(LightType a_lightType, Camera a_camera);

        public abstract void Resize(uint a_width, uint a_height);
        public abstract void PostProcess(Camera a_camera);

        public static void Init(RenderPipeline a_pipeline)
        {
            if (Instance is IDisposable disp)
            {
                disp.Dispose();
            }

            Instance = a_pipeline;
        }
        internal static void Destroy()
        {
            if (Instance is IDisposable disp)
            {
                disp.Dispose();
                Instance = null;
            }
        }
        
        static void PreShadowS(uint a_camBuffer)
        {
            if (Instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);

                if (cam != null)
                {
                    Instance.PreShadow(cam);
                }
            }
            else
            {
                Logger.FlareError("RenderPipeline not initialized");
            }
        }
        static void PostShadowS(uint a_camBuffer)
        {
            if (Instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);

                if (cam != null)
                {
                    Instance.PostShadow(cam);
                }
            }
            else
            {
                Logger.FlareError("RenderPipeline not initialized");
            } 
        }

        static void PreRenderS(uint a_camBuffer)
        {
            if (Instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);

                if (cam != null)
                {
                    Instance.PreRender(cam);
                }
            }
            else
            {
                Logger.FlareError("RenderPipeline not initialized");
            }
        }
        static void PostRenderS(uint a_camBuffer)
        {
            if (Instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);

                if (cam != null)
                {
                    Instance.PostRender(cam);
                }
            }
            else
            {
                Logger.FlareError("RenderPipeline not initialized");
            }
        }

        static void PreLightS(uint a_lightType, uint a_camBuffer)
        {
            if (Instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);

                if (cam != null)
                {
                    Material mat = Instance.PreLight((LightType)a_lightType, cam);

                    if (mat != null)
                    {
                        RenderCommand.BindMaterial(mat);
                    }
                }
            }
            else
            {
                Logger.FlareError("RenderPipeline not initialized");
            }
        }
        static void PostLightS(uint a_lightType, uint a_camBuffer)
        {
            if (Instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);

                if (cam != null)
                {
                    Instance.PostLight((LightType)a_lightType, cam);
                }
            }
            else
            {
                Logger.FlareError("RenderPipelien not initialized");
            }
        }

        static void PostProcessS(uint a_camBuffer)
        {
            if (Instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);

                if (cam != null)
                {
                    Instance.PostProcess(cam);
                }
            }
            else
            {
                Logger.FlareError("RenderPipeline not initialized");
            }
        }

        static void ResizeS(uint a_width, uint a_height)
        {
            if (Instance != null)
            {
                Instance.Resize(a_width, a_height);
            }
            else
            {
                Logger.FlareError("RenderPipeline not initialized");
            }
        }
    }
}