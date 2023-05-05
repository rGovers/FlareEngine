using FlareEngine.Definitions;
using FlareEngine.Maths;
using FlareEngine.Mod;
using System;
using System.Collections.Generic;
using System.Xml;

namespace FlareEngine
{
    public class SceneObject
    {
        public Vector3 Translation;
        public Quaternion Rotation;
        public Vector3 Scale;
        public string DefName;
    }

    public class Scene : IDestroy
    {
        bool              m_disposed = false;

        List<Def>         m_defs;

        List<SceneObject> m_sceneObjects;
        List<GameObject>  m_objects;

        public bool IsDisposed 
        {
            get
            {
                return m_disposed;
            }
        }

        public IEnumerable<Def> Defs
        {
            get
            {
                return m_defs;
            }
        }

        public IEnumerable<GameObject> GameObjects
        {
            get
            {
                return m_objects;
            }
        }

        public IEnumerable<SceneObject> SceneObjects
        {
            get
            {
                return m_sceneObjects;
            }
        }

        void LoadSceneObject(XmlElement a_element)
        {
            SceneObject obj = new SceneObject();
            obj.Translation = Vector3.Zero;
            obj.Rotation = Quaternion.Identity;
            obj.Scale = Vector3.One;

            foreach (XmlElement node in a_element.ChildNodes)
            {
                if (node is XmlElement element)
                {
                    switch (element.Name)
                    {
                    case "Translation":
                    {
                        obj.Translation = element.ToVector3();

                        break;
                    }
                    case "Rotation":
                    {
                        obj.Rotation = Quaternion.Normalized(element.ToQuaternion());

                        break;
                    }
                    case "AxisAngle":
                    {
                        Vector4 rot = element.ToVector4(Vector4.Zero);

                        obj.Rotation = Quaternion.FromAxisAngle(Vector3.Normalized(rot.XYZ), rot.W);

                        break;
                    }
                    case "Scale":
                    {
                        obj.Scale = element.ToVector3(Vector3.One);

                        break;
                    }
                    case "DefName":
                    {
                        obj.DefName = element.InnerText;

                        break;
                    }
                    default:
                    {
                        Logger.FlareError($"Invalid Scene element: {element.Name}");

                        return;
                    }
                    }
                }
            }

            if (!string.IsNullOrWhiteSpace(obj.DefName))
            {
                m_sceneObjects.Add(obj);
            }
            else
            {
                Logger.FlareWarning($"Invalid Scene Object");
            }
        }

        void LoadObjects(XmlElement a_element)
        {
            foreach (XmlNode node in a_element.ChildNodes)
            {
                if (node is XmlElement element)
                {
                    if (element.Name == "GameObject")
                    {
                        LoadSceneObject(element);
                    }
                    else
                    {
                        Logger.FlareError($"Invalid Scene element: {element.Name}");

                        return;
                    }
                }                
            }
        }
        void LoadDefs(XmlElement a_element)
        {
            List<DefData> data = new List<DefData>();

            foreach (XmlNode node in a_element.ChildNodes)
            {
                if (node is XmlElement element)
                {
                    data.Add(DefLibrary.GetDefData(Def.SceneDefPath, element));
                }
            }

            DefLibrary.LoadSceneDefs(data);

            DefLibrary.ResolveSceneDefs();

            foreach (DefData dat in data)
            {
                Def def = DefLibrary.GetDef(dat.Name);

                if (def != null)
                {
                    m_defs.Add(def);
                }
            }
        }

        public Scene(XmlDocument a_doc)
        {
            m_defs = new List<Def>();
            m_sceneObjects = new List<SceneObject>();
            m_objects = new List<GameObject>();

            if (a_doc.DocumentElement is XmlElement root)
            {
                foreach (XmlNode node in root.ChildNodes)
                {
                    if (node is XmlElement element)
                    {
                        switch (element.Name)
                        {
                        case "Objects":
                        {
                            LoadObjects(element);

                            break;
                        }
                        case "Defs":
                        {
                            LoadDefs(element);

                            break;
                        }
                        default:
                        {
                            Logger.FlareError($"Invalid scene element {element.Name}");

                            break;
                        }
                        }
                    }                    
                }
            }
        }

        public static Scene LoadScene(string a_path)
        {
            string path = ModControl.GetAssetPath(a_path);

            if (string.IsNullOrEmpty(path))
            {
                Logger.FlareError("Scene not found");

                return null;
            }

            XmlDocument doc = new XmlDocument();
            doc.Load(path);

            return new Scene(doc);
        }

        void Dispose(bool a_disposing)
        {
            if (!m_disposed)
            {
                if (a_disposing)
                {
                    foreach (GameObject obj in m_objects)
                    {
                        if (obj != null && !obj.IsDisposed)
                        {
                            obj.Dispose();
                        }
                    }

                    m_objects.Clear();
                }
                else
                {
                    Logger.FlareError("Scene Failed to Dispose");
                }

                m_disposed = true;
            }
            else
            {
                Logger.FlareError("Scene Multiple Dispose");
            }
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }
        ~Scene()
        {
            Dispose(false);
        }
    }
}