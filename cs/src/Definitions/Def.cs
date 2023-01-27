using System;

namespace FlareEngine.Definitions
{
    public class Def
    {
        [HideInEditor, NonSerialized]
        public string DefName = string.Empty;
        [HideInEditor, NonSerialized]
        public string DefPath = string.Empty;
        [HideInEditor, NonSerialized]
        public string DefParentName = string.Empty;

        public virtual void PostResolve() { }
    };
}