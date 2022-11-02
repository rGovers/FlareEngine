using FlareEngine.Rendering;
using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Xml;

namespace FlareEngine.Definitions
{
    internal struct DefDataObject
    {
        public string Name;
        public string Text;
        public List<DefDataObject> Children;
    }

    internal struct DefData
    {
        public string Type;
        public string Path;
        public string Name;
        public string Parent;
        public bool Abstract;
        public List<DefDataObject> DefDataObjects;
    }

    public static class DefLibrary
    {
        static List<Def>               m_defs;

        static Dictionary<string, Def> m_defLookup;

        static DefDataObject? GetData(XmlElement a_element)
        {
            DefDataObject dataObj;
            dataObj.Name = a_element.Name;
            dataObj.Text = a_element.InnerText;
            dataObj.Children = new List<DefDataObject>();

            foreach (XmlNode node in a_element.ChildNodes)
            {
                XmlElement element = node as XmlElement;
                if (element != null)
                {
                    DefDataObject? data = GetData(element);
                    if (data != null)
                    {
                        dataObj.Children.Add(data.Value);
                    }
                    else
                    {
                        return null;
                    }
                }
            }

            if (string.IsNullOrWhiteSpace(dataObj.Text) && dataObj.Children.Count <= 0)
            {
                Console.Error.WriteLine("FlareCS: Invalid Def DataObject: " + dataObj.Name);

                return null;
            }

            return dataObj;
        }

        static void LoadDefData(string a_path, ref List<DefData> a_data)
        {
            string[] files = Directory.GetFiles(a_path);

            foreach (string file in files)
            {
                FileInfo info = new FileInfo(file);

                if (info.Extension == ".xml")
                {
                    XmlDocument doc = new XmlDocument();
                    doc.Load(file);
                    
                    XmlElement root = doc.DocumentElement as XmlElement;
                    if (root != null)
                    {
                        DefData data;
                        data.Type = root.Name;
                        data.Path = file;
                        data.Name = string.Empty;
                        data.Parent = string.Empty;
                        data.Abstract = false;
                        data.DefDataObjects = new List<DefDataObject>();

                        foreach (XmlAttribute att in root.Attributes)
                        {
                            switch (att.Name)
                            {
                            case "Name":
                            {
                                data.Name = att.Value;

                                break;
                            }
                            case "Parent":
                            {
                                data.Parent = att.Value;

                                break;
                            }
                            case "Abstract":
                            {
                                bool val;
                                if (Boolean.TryParse(att.Value, out val))
                                {
                                    data.Abstract = val;
                                }
                                else
                                {
                                    Console.Error.WriteLine("FlareCS: Error parsing Abstract value: " + att.Value + " : " + file);
                                }

                                break;
                            }
                            default:
                            {
                                Console.Error.WriteLine("FlareCS: Invalid Def Attribute: " + att.Name + " : " + file);

                                break;
                            }
                            }
                        }

                        foreach (XmlNode node in root.ChildNodes)
                        {
                            XmlElement element = node as XmlElement;
                            if (element != null)
                            {
                                DefDataObject? dataObject = GetData(element);
                                if (dataObject != null)
                                {
                                    data.DefDataObjects.Add(dataObject.Value);
                                }
                            }
                        }

                        if (!string.IsNullOrWhiteSpace(data.Name))
                        {
                            a_data.Add(data);
                        }
                        else
                        {
                            Console.Error.WriteLine("FlareCS: Error parsing unamed Def: " + file);
                        }
                    }
                }
            }

            string[] dirs = Directory.GetDirectories(a_path);
            foreach (string dir in dirs)
            {
                LoadDefData(a_path, ref a_data);
            }
        }

        static bool SetDefData(Def a_def, DefData a_data, List<DefData> a_dataList)
        {
            if (!string.IsNullOrWhiteSpace(a_data.Parent))
            {
                bool found = false;
                foreach (DefData dat in a_dataList)
                {
                    if (dat.Name == a_data.Name)
                    {
                        if (!SetDefData(a_def, dat, a_dataList))
                        {
                            return false;
                        }

                        found = true;

                        break;
                    }
                }

                if (!found)
                {
                    Console.Error.WriteLine("FlareCS: Cannot find def parent: " + a_data.Parent + ", " + a_data.Name + " : " + a_data.Path);

                    return false;
                }
            }

            a_def.DefName = a_data.Name;

            Type type = a_def.GetType();

            foreach (DefDataObject obj in a_data.DefDataObjects)
            {
                FieldInfo field = type.GetField(obj.Name, BindingFlags.Public | BindingFlags.Instance);
                if (field != null)
                {
                    Type fieldType = field.FieldType;
                    
                    if (fieldType == typeof(string))
                    {
                        field.SetValue(a_def, obj.Text);
                    }
                    else if (fieldType == typeof(uint))
                    {
                        uint val;
                        if (uint.TryParse(obj.Text, out val))
                        {
                            field.SetValue(a_def, val);
                        }
                        else
                        {
                            Console.Error.WriteLine("FlareCS: Cannot Parse Def uint: " + obj.Name + ", " + a_data.Name + " : " + a_data.Path);
                        }
                    }
                    else if (fieldType == typeof(int))
                    {
                        int val;
                        if (int.TryParse(obj.Text, out val))
                        {
                            field.SetValue(a_def, val);
                        }
                        else
                        {
                            Console.Error.WriteLine("FlareCS: Cannot Parse Def int: " + obj.Name + ", " + a_data.Name + " : " + a_data.Path);
                        }
                    }
                    else if (fieldType == typeof(VertexShader))
                    {
                        field.SetValue(a_def, AssetLibrary.LoadVertexShader(obj.Text));
                    }
                    else if (fieldType == typeof(PixelShader))
                    {
                        field.SetValue(a_def, AssetLibrary.LoadPixelShader(obj.Text));
                    }
                    else
                    {
                        Console.Error.WriteLine("FlareCS: Invalid Def Field Type: " + obj.Name + ", " + a_data.Name + " : " + a_data.Path);
                    }
                }   
                else
                {   
                    Console.Error.WriteLine("FlareCS: Invalid Def Field: " + obj.Name + ", " + a_data.Name + " : " + a_data.Path);
                }
            }
            
            return true;
        }

        static Def CreateDef(DefData a_data, List<DefData> a_dataList)
        {
            Type type = Type.GetType(a_data.Type, false, false);
            if (type == null)
            {
                type = Type.GetType("FlareEngine.Definitions." + a_data.Type, false, false);
            }

            if (type != null)
            {
                Def defObj = null;
                if (m_defLookup.ContainsKey(a_data.Name))
                {
                    defObj = m_defLookup[a_data.Name];
                }
                else
                {
                    defObj = Activator.CreateInstance(type) as Def;
                }

                if (defObj != null)
                {
                    if (!SetDefData(defObj, a_data, a_dataList))
                    {
                        return null;
                    }

                    return defObj;
                }
                else
                {
                    Console.Error.WriteLine("FlareCS: Error creating Def: " + a_data.Type + ", " + a_data.Name + " : " + a_data.Path);
                }
            }
            else
            {
                Console.Error.WriteLine("FlareCS: Invalid Def Type: " + a_data.Type + ", " + a_data.Name + " : " + a_data.Path);
            }

            return null;
        }

        internal static void Init()
        {
            m_defs = new List<Def>();
            m_defLookup = new Dictionary<string, Def>();
        }

        public static void Clear()
        {
            m_defs.Clear();
            m_defLookup.Clear();
        }

        public static void LoadDefs(string a_path)
        {
            if (Directory.Exists(a_path))
            {
                Console.WriteLine("FlareCS: Loading Defs");

                List<DefData> defData = new List<DefData>();
                LoadDefData(a_path, ref defData);

                Console.WriteLine("FlareCS: Building DefTable");

                foreach (DefData dat in defData)
                {
                    if (dat.Abstract)
                    {
                        continue;
                    }

                    Def def = CreateDef(dat, defData); 
                    if (def != null)
                    {
                        m_defs.Add(def);
                        m_defLookup.Add(def.DefName, def);
                    }
                }
            }
        }

        public static List<T> GetDefs<T>() where T : Def
        {
            List<T> defs = new List<T>();
            
            foreach (Def def in m_defs)
            {
                if (def is T)
                {
                    defs.Add((T)def);
                }
            }

            return defs;
        }
        public static Def GetDef(string a_name)
        {
            if (m_defLookup.ContainsKey(a_name))
            {
                return m_defLookup[a_name];
            }

            return null;
        }
        public static T GetDef<T>(string a_name) where T : Def
        {
            if (m_defLookup.ContainsKey(a_name))
            {
                Def def = m_defLookup[a_name];
                if (def is T)
                {
                    return (T)def;
                }
            }

            return null;
        }
    };
}