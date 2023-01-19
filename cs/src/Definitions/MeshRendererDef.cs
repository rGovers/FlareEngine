using FlareEngine.Rendering;

namespace FlareEngine.Definitions
{
    public class MeshRendererDef : ComponentDef
    {
        [EditorTooltip("Path relative to the project for the model file to be used.")]
        public string ModelPath = null;

        [EditorTooltip("The material to use for rendering.")]
        public MaterialDef MaterialDef = null;

        public override void PostResolve()
        {
            base.PostResolve();

            if (ComponentType == null || !ComponentType.IsSubclassOf(typeof(MeshRenderer)))
            {
                Logger.Error("FlareCS: Component Def Invalid ComponentType");

                return;
            }

            if (string.IsNullOrEmpty(ModelPath))
            {
                Logger.Warning("FlareCS: Component Def Invalid ModelPath");
            }

            if (MaterialDef == null)
            {
                Logger.Warning("FlareCS: Component Def Invalid Material");
            }
        }
    }
}