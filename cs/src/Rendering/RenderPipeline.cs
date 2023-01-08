namespace FlareEngine.Rendering
{
    public class RenderPipeline
    {
        static RenderPipeline Instance = new RenderPipeline();

        public virtual void PreShadow(Camera a_camera) { }
        public virtual void PostShadow(Camera a_camera) { }

        public virtual void PreRender(Camera a_camera) { }
        public virtual void PostRender(Camera a_camera) { } 

        public virtual void PostProcess(Camera a_camera) { } 
        
        static void PreShadow(uint a_camBuffer)
        {
            Camera cam = Camera.GetCamera(a_camBuffer);

            if (cam != null)
            {
                Instance.PreShadow(cam);
            }
        }
        static void PostShadow(uint a_camBuffer)
        {
            Camera cam = Camera.GetCamera(a_camBuffer);

            if (cam != null)
            {
                Instance.PostShadow(cam);
            }
        }

        static void PreRender(uint a_camBuffer)
        {
            Camera cam = Camera.GetCamera(a_camBuffer);

            if (cam != null)
            {
                Instance.PreRender(cam);
            }
        }
        static void PostRender(uint a_camBuffer)
        {
            Camera cam = Camera.GetCamera(a_camBuffer);

            if (cam != null)
            {
                Instance.PostRender(cam);
            }
        }

        static void PostProcess(uint a_camBuffer)
        {
            Camera cam = Camera.GetCamera(a_camBuffer);

            if (cam != null)
            {
                Instance.PostProcess(cam);
            }
        }
    }
}