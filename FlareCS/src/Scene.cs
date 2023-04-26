using FlareEngine.Definitions;
using FlareEngine.Maths;
using FlareEngine.Mod;
using System;
using System.Collections.Generic;
using System.Xml;

namespace FlareEngine
{
    public struct SceneObject
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

        string            m_name;
        List<SceneObject> m_sceneObjects;
        List<GameObject>  m_objects;

        public string Name
        {
            get
            {
                return m_name;
            }
        }

        public bool IsDisposed 
        {
            get
            {
                return m_disposed;
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

        void LoadObjects(XmlElement a_element)
        {
            foreach (XmlElement element in a_element.ChildNodes)
            {
                if (element.Name == "GameObject")
                {
                    SceneObject obj = new SceneObject();

                    foreach (XmlElement oElement in element.ChildNodes)
                    {
                        switch (oElement.Name)
                        {
                        case "Transform":
                        {
                            foreach (XmlElement tElement in oElement.ChildNodes)
                            {
                                switch (tElement.Name)
                                {
                                case "Translation":
                                {
                                    obj.Translation = tElement.ToVector3();

                                    break;
                                }
                                case "Rotation":
                                {
                                    obj.Rotation = tElement.ToQuaternion();

                                    break;
                                }
                                case "Scale":
                                {
                                    obj.Scale = tElement.ToVector3(Vector3.One);

                                    break;
                                }
                                }
                            }

                            break;
                        }
                        case "DefName":
                        {
                            obj.DefName = oElement.InnerText;

                            break;
                        }
                        default:
                        {
                            Logger.FlareError($"Invalid Scene element {oElement.Name}");

                            return;
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
                else
                {
                    Logger.FlareError($"Invalid Scene element {element.Name}");

                    return;
                }
            }
        }
        void LoadDefs(XmlElement a_element)
        {
            List<DefData> data = new List<DefData>();

            foreach (XmlElement element in a_element.ChildNodes)
            {
                data.Add(DefLibrary.GetDefData("[Scene]", element));
            }

            DefLibrary.LoadDefs(data);

            DefLibrary.ResolveDefs();
        }

        public Scene(XmlDocument a_doc)
        {
            m_defs = new List<Def>();
            m_sceneObjects = new List<SceneObject>();
            m_objects = new List<GameObject>();

            if (a_doc.DocumentElement is XmlElement root)
            {
                m_name = root.GetAttribute("Name");

                foreach (XmlElement element in root.ChildNodes)
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