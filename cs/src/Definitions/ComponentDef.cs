using System;

namespace FlareEngine.Definitions
{
    public class ComponentDef : Def
    {
        public Type ComponentType = typeof(Component);

        public override void PostResolve()
        {
            base.PostResolve();

            if (ComponentType == null || !ComponentType.IsSubclassOf(typeof(Component)))
            {
                Logger.Error("FlareCS: Component Def Invalid ComponentType");

                return;
            }
        }
    }
}