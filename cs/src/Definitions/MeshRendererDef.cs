using FlareEngine.Rendering;
using System;

namespace FlareEngine.Definitions
{
    public class MeshRendererDef : ComponentDef
    {
        public string ModelPath = null;

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