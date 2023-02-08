using System;

namespace FlareEngine.Rendering
{
    public class RenderPipeline : IDisposable
    {
        static RenderPipeline Instance = null;

        IRenderTexture m_renderTexture;

        public virtual void PreShadow(Camera a_camera) { }
        public virtual void PostShadow(Camera a_camera) { }

        public virtual void PreRender(Camera a_camera) 
        { 
            RenderCommand.BindRenderTexture(null);
        }
        public virtual void PostRender(Camera a_camera) 
        { 

        } 

        public virtual void Resize(uint a_width, uint a_height)
        {
            m_renderTexture.Resize(a_width, a_height);
        }

        public virtual void PostProcess(Camera a_camera) { } 
        
        public RenderPipeline()
        {
            // m_renderTexture = new MultiRenderTexture(5, 1920, 1080, true);
            m_renderTexture = new RenderTexture(1920, 1080, true);
        }

        public virtual void Dispose()
        {
            m_renderTexture.Dispose();
        }

        public static void Init(RenderPipeline a_pipeline)
        {
            if (Instance != null)
            {
                Instance.Dispose();
            }

            Instance = a_pipeline;
        }
        internal static void Destroy()
        {
            if (Instance != null)
            {
                Instance.Dispose();
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