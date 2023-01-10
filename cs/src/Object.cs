using FlareEngine.Definitions;
using System;
using System.Collections.Generic;

namespace FlareEngine
{
    public class Object : IDisposable
    {
        static List<Object> Objs = new List<Object>();
        static Dictionary<string, Object> ObjDictionary = new Dictionary<string, Object>();

        ObjectDef       m_def = null;

        List<Component> m_components;

        string          m_tag = null;

        bool            m_disposed = false;

        Transform       m_transform;

        public ObjectDef Def
        {
            get
            {
                return m_def;
            }
        }

        public string Tag
        {
            get
            {
                return m_tag;
            }
        }

        public Transform Transform
        {
            get
            {
                return m_transform;
            }
        }

        public Object Parent 
        {
            get
            {
                return m_transform.Parent.Object;
            }
            set
            {
                m_transform.Parent = value.m_transform;
            }
        }

        public IEnumerable<Component> Components
        {
            get
            {
                return m_components;
            }
        }

        public Object()
        {
            m_transform = new Transform(this);

            m_components = new List<Component>();
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool a_disposing)
        {
            if(!m_disposed)
            {
                if(a_disposing)
                {
                    m_transform.Dispose();
                    m_transform = null;   

                    Objs.Remove(this);

                    if (!string.IsNullOrWhiteSpace(m_tag) && ObjDictionary.ContainsKey(m_tag))
                    {
                        ObjDictionary.Remove(m_tag);
                    }

                    foreach (Component comp in m_components)
                    {
                        if (comp is IDisposable val)
                        {
                            val.Dispose();
                        }
                    }
                    m_components.Clear();
                }
                else
                {
                    Logger.Error("FlareCS: Object Failed to Dispose");
                }

                m_disposed = true;
            }
            else
            {
                Logger.Error("FlareCS: Object Failed to Dispose");
            }
        }

        ~Object()
        {
            Dispose(false);
        }

        internal static void UpdateObjects()
        {
            Profiler.StartFrame("Object Update");

            foreach (Object obj in Objs)
            {
                obj.Update();
            }

            Profiler.StopFrame();
        }

        public virtual void Init() { }
        public virtual void Update() { }

        public T AddComponent<T>() where T : Component
        {
            T comp = Activator.CreateInstance<T>();
            comp.Object = this;

            if (comp != null)
            {
                m_components.Add(comp);
            }

            comp.Init();

            return comp; 
        }
        public Component AddComponent(ComponentDef a_def)
        {
            return AddComponent<Component>(a_def);
        }
        public T AddComponent<T>(ComponentDef a_def) where T : Component
        {
            T comp = Component.FromDef<T>(a_def);
            comp.Object = this;

            if (comp != null)
            {
                m_components.Add(comp);
            }

            comp.Init();

            return comp;
        }

        public T GetComponent<T>() where T : Component
        {
            foreach (Component comp in m_components)
            {
                if (comp is T val)
                {
                    return val;
                }
            }

            return null;
        }
        public Component GetComponent(ComponentDef a_def)
        {
            return GetComponent<Component>(a_def);
        }
        public T GetComponent<T>(ComponentDef a_def) where T : Component
        {
            foreach (Component comp in m_components)
            {
                if (comp.Def == a_def)
                {
                    return comp as T;
                }
            }

            return null;
        }

        public void RemoveComponent(Component a_component)
        {
            m_components.Remove(a_component);
        }
        public void RemoveComponent(ComponentDef a_def)
        {
            foreach (Component comp in m_components)
            {
                if (comp.Def == a_def)
                {
                    RemoveComponent(comp);

                    if (comp is IDisposable val)
                    {
                        val.Dispose();
                    }

                    return;
                }
            }
        }

        public static Object Instantiate(string a_tag = null)
        {
            Object obj = new Object()
            {
                m_tag = a_tag
            };

            if (!string.IsNullOrEmpty(a_tag))
            {
                ObjDictionary.Add(a_tag, obj);
            }

            Objs.Add(obj);

            obj.Init();

            return obj;
        }
        public static T Instantiate<T>(string a_tag = null) where T : Object
        {
            T obj = Activator.CreateInstance<T>();
            obj.m_tag = a_tag;

            if (!string.IsNullOrEmpty(a_tag))
            {
                ObjDictionary.Add(a_tag, obj);
            }

            Objs.Add(obj);

            obj.Init();

            return obj;
        }   

        public static Object FromDef(ObjectDef a_def, string a_tag = null)
        {
            return FromDef<Object>(a_def, a_tag);
        }
        public static T FromDef<T>(ObjectDef a_def, string a_tag = null) where T : Object
        {
            T obj = Activator.CreateInstance(a_def.ObjectType) as T;

            obj.m_def = a_def;

            if (!string.IsNullOrEmpty(a_tag))
            {
                ObjDictionary.Add(a_tag, obj);
            }

            Objs.Add(obj);

            obj.Init();

            return obj;
        }
    }
}