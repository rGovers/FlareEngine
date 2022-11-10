using System;
using System.Collections.Generic;

namespace FlareEngine.Definitions
{
    public class ObjectDef : Def
    {
        public Type ObjectType = typeof(Object);

        public List<ComponentDef> Components = new List<ComponentDef>();

        public override void PostResolve()
        {
            base.PostResolve();

            if (ObjectType == null || !ObjectType.IsSubclassOf(typeof(Object)))
            {
                Logger.Error("FlareCS: Object Def Invalid ObjectType");

                return;
            }
        }
    }
}