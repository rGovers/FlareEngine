using FlareEngine.Rendering.Lighting;
using System;

namespace FlareEngine.Rendering
{
    public class DefaultRenderPipeline : RenderPipeline, IDisposable
    {
        MultiRenderTexture m_drawRenderTexture;
        RenderTexture      m_lightRenderTexture;

        TextureSampler     m_colorSampler;
        TextureSampler     m_normalSampler;
        TextureSampler     m_specularSampler;
        TextureSampler     m_emissionSampler;
        TextureSampler     m_depthSampler;

        TextureSampler     m_lightColorSampler;

        void SetTextures(Material a_mat)
        {
            a_mat.SetTexture(0, m_colorSampler);
            a_mat.SetTexture(1, m_normalSampler);
            a_mat.SetTexture(2, m_specularSampler);
            a_mat.SetTexture(3, m_emissionSampler);
            a_mat.SetTexture(4, m_depthSampler);
        }

        void SetPostTextures()
        {
            Material postMat = Material.PostMaterial;

            postMat.SetTexture(0, m_lightColorSampler);
            postMat.SetTexture(1, m_normalSampler);
            postMat.SetTexture(2, m_emissionSampler);
            postMat.SetTexture(3, m_depthSampler);
        }

        public DefaultRenderPipeline()
        {
            m_drawRenderTexture = new MultiRenderTexture(4, 1920, 1080, true, true);
            m_lightRenderTexture = new RenderTexture(1920, 1080, false, true);

            m_colorSampler = TextureSampler.GenerateRenderTextureSampler(m_drawRenderTexture, 0);
            m_normalSampler = TextureSampler.GenerateRenderTextureSampler(m_drawRenderTexture, 1);
            m_specularSampler = TextureSampler.GenerateRenderTextureSampler(m_drawRenderTexture, 2);
            m_emissionSampler = TextureSampler.GenerateRenderTextureSampler(m_drawRenderTexture, 3);
            m_depthSampler = TextureSampler.GenerateRenderTextureDepthSampler(m_drawRenderTexture);

            m_lightColorSampler = TextureSampler.GenerateRenderTextureSampler(m_lightRenderTexture);

            SetTextures(Material.DirectionalLightMaterial);
            SetTextures(Material.PointLightMaterial);
            SetTextures(Material.SpotLightMaterial);

            SetPostTextures();
        }

        public override void Resize(uint a_width, uint a_height)
        {
            m_drawRenderTexture.Resize(a_width, a_height);
            m_lightRenderTexture.Resize(a_width, a_height);

            SetTextures(Material.DirectionalLightMaterial);
            SetTextures(Material.PointLightMaterial);
            SetTextures(Material.SpotLightMaterial);

            SetPostTextures();
        }

        public override void PreShadow(Camera a_camera) 
        {

        }
        public override void PostShadow(Camera a_camera)
        {
            
        }

        public override void PreRender(Camera a_camera)
        {
            RenderCommand.BindRenderTexture(m_drawRenderTexture);
        }
        public override void PostRender(Camera a_camera)
        {
            
        }

        public override void LightSetup(Camera a_camera)
        {
            RenderCommand.BindRenderTexture(m_lightRenderTexture);
        }

        public override Material PreLight(LightType a_lightType, Camera a_camera)
        {
            Material mat = null;

            switch (a_lightType)
            {
            case LightType.Directional:
            {
                mat = Material.DirectionalLightMaterial;
                
                break;
            }
            case LightType.Point:
            {
                mat = Material.PointLightMaterial;

                break;
            }
            case LightType.Spot:
            {
                mat = Material.SpotLightMaterial;

                break;
            }
            }   
            
            return mat;
        }
        public override void PostLight(LightType a_lightType, Camera a_camera)
        {
            
        }

        public override void PostProcess(Camera a_camera)
        {
            RenderCommand.BindRenderTexture(a_camera.RenderTexture);
            RenderCommand.BindMaterial(Material.PostMaterial);

            RenderCommand.DrawMaterial();
        }

        public virtual void Dispose()
        {
            m_drawRenderTexture.Dispose();
            m_lightRenderTexture.Dispose();

            m_colorSampler.Dispose();
            m_normalSampler.Dispose();
            m_specularSampler.Dispose();
            m_emissionSampler.Dispose();
            m_depthSampler.Dispose();

            m_lightColorSampler.Dispose();
        }
    }
}