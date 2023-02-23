using FlareEngine.Rendering.Lighting;
using System;

namespace FlareEngine.Rendering
{
    public class DefaultRenderPipeline : RenderPipeline, IDisposable
    {
        MultiRenderTexture m_renderTexture;

        TextureSampler     m_colorSampler;
        TextureSampler     m_normalSampler;

        public DefaultRenderPipeline()
        {
            m_renderTexture = new MultiRenderTexture(4, 1920, 1080, true, true);

            m_colorSampler = TextureSampler.GenerateRenderTextureSampler(m_renderTexture, 0);
            m_normalSampler = TextureSampler.GenerateRenderTextureSampler(m_renderTexture, 1);

            Material.DirectionalLightMaterial.SetTexture(0, m_colorSampler);
            Material.DirectionalLightMaterial.SetTexture(1, m_normalSampler);
        }

        public override void Resize(uint a_width, uint a_height)
        {
            m_renderTexture.Resize(a_width, a_height);

            Material.DirectionalLightMaterial.SetTexture(0, m_colorSampler);
            Material.DirectionalLightMaterial.SetTexture(1, m_normalSampler);
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

            m_colorSampler.Dispose();
            m_normalSampler.Dispose();
        }
    }
}