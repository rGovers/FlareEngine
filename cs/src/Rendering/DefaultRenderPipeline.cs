using FlareEngine.Rendering.Lighting;
using System;

namespace FlareEngine.Rendering
{
    public class DefaultRenderPipeline : RenderPipeline, IDisposable
    {
        IRenderTexture m_renderTexture;

        public DefaultRenderPipeline()
        {
            m_renderTexture = new MultiRenderTexture(4, 1920, 1080, true, true);
        }

        public override void Resize(uint a_width, uint a_height)
        {
            m_renderTexture.Resize(a_width, a_height);
        }

        public override void PreShadow(Camera a_camera) 
        {

        }
        public override void PostShadow(Camera a_camera)
        {
            
        }

        public override void PreRender(Camera a_camera)
        {
            RenderCommand.BindRenderTexture(m_renderTexture);
        }
        public override void PostRender(Camera a_camera)
        {
            
        }

        public override Material PreLight(LightType a_lightType, Camera a_camera)
        {
            switch (a_lightType)
            {
            case LightType.Directional:
            {
                return Material.DirectionalLightMaterial;
            }
            }   

            return null;
        }
        public override void PostLight(LightType a_lightType, Camera a_camera)
        {
            
        }

        public override void PostProcess(Camera a_camera)
        {
            // RenderCommand.Blit(m_renderTexture, null);
        }

        public virtual void Dispose()
        {
            m_renderTexture.Dispose();
        }
    }
}