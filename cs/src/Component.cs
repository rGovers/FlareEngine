using FlareEngine.Definitions;
using System;

namespace FlareEngine
{
    public class Component 
    {
        ComponentDef m_def = null;

        GameObject   m_object;

        public ComponentDef Def
        {
            get
            {
                return m_def;
            }
        }

        public GameObject Object
        {
            get
            {
                return m_object;
            }
            internal set
            {
                m_object = value;
            }
        }

        public Transform Transform
        {
            get
            {
                return m_object.Transform;
            }
        }

        internal Component() { }

        public virtual void Init() { }

        internal static T FromDef<T>(ComponentDef a_def) where T : Component
        {
            T comp = Activator.CreateInstance(a_def.ComponentType) as T;

            if (comp != null)
            {
                comp.m_def = a_def;

                comp.Init();
            }

            return comp;
        }
    }
}