using FlareEngine.Maths;
using System;
using System.Collections;
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
                if (node is XmlElement element)
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
                Logger.Error("FlareCS: Invalid Def DataObject: " + dataObj.Name);

                return null;
            }

            return dataObj;
        }

        static void DefError(Type a_type, DefDataObject a_datObj, DefData a_data)
        {
            Logger.Error("FlareCS: Cannot Parse Def " + a_type.ToString() + ": " + a_datObj.Name + ", " + a_data.Name + " : " + a_data.Path);
        }

        static void LoadDefVariables(object a_obj, DefDataObject a_datObj, DefData a_data)
        {
            Type type = a_obj.GetType();

            FieldInfo field = type.GetField(a_datObj.Name, BindingFlags.Public | BindingFlags.Instance);
            if (field != null)
            {
                object obj = field.GetValue(a_obj);

                switch (obj)
                {
                case Type _:
                {
                    Type val = Type.GetType(a_datObj.Text, false);
                    if (val != null)
                    {
                        field.SetValue(a_obj, val);
                    }
                    else
                    {
                        DefError(typeof(Type), a_datObj, a_data);
                    }

                    break;
                }
                case uint val:
                {
                    if (uint.TryParse(a_datObj.Text, out val))
                    {
                        field.SetValue(a_obj, val);
                    }
                    else
                    {
                        DefError(typeof(uint), a_datObj, a_data);
                    }

                    break;
                }
                case int val:
                {
                    if (int.TryParse(a_datObj.Text, out val))
                    {
                        field.SetValue(a_obj, val);
                    }
                    else
                    {
                        DefError(typeof(int), a_datObj, a_data);
                    }

                    break;
                }
                case float val:
                {
                    if (float.TryParse(a_datObj.Text, out val))
                    {
                        field.SetValue(a_obj, val);
                    }
                    else
                    {
                        DefError(typeof(float), a_datObj, a_data);
                    }

                    break;
                }
                default:
                {
                    Type fieldType = field.FieldType;

                    if (fieldType.IsSubclassOf(typeof(Def)))
                    {
                        Def def = new Def()
                        {
                            DefName = a_datObj.Name
                        };
                        
                        field.SetValue(a_obj, def);
                    }
                    else if (fieldType == typeof(string))
                    {
                        field.SetValue(a_obj, a_datObj.Text);
                    }
                    else if (fieldType.IsSubclassOf(typeof(Enum)))
                    {
                        int val;
                        if (int.TryParse(a_datObj.Text, out val))
                        {
                            if (Enum.IsDefined(fieldType, val))
                            {
                                field.SetValue(a_obj, val);
                            }
                            else
                            {
                                DefError(fieldType, a_datObj, a_data);
                            }
                        }   
                        else
                        {
                            object eVal = Enum.Parse(fieldType, a_datObj.Text);
                            if (eVal != null)
                            {
                                field.SetValue(a_obj, eVal);
                            }
                            else
                            {
                                DefError(fieldType, a_datObj, a_data);
                            }
                        }
                    }
                    else if (fieldType.IsGenericType && fieldType.GetGenericTypeDefinition() == typeof(List<>))
                    {
                        Type genericType = fieldType.GetGenericArguments()[0];
                        MethodInfo methodInfo = fieldType.GetMethod("Add");

                        if (obj == null)
                        {
                            obj = Activator.CreateInstance(fieldType);
                        } 

                        foreach (DefDataObject datObj in a_datObj.Children)
                        {
                            if (datObj.Name == "lv")
                            {
                                object listObj = Activator.CreateInstance(genericType);

                                foreach (DefDataObject objVal in datObj.Children)
                                {
                                    LoadDefVariables(listObj, objVal, a_data);
                                }
                                
                                methodInfo.Invoke(obj, new object[] { listObj });
                            }
                            else
                            {
                                DefError(fieldType, a_datObj, a_data);
                            }
                        }

                        field.SetValue(a_obj, obj);
                    }
                    else
                    {
                        obj = Activator.CreateInstance(fieldType);

                        foreach (DefDataObject objVal in a_datObj.Children)
                        {
                            LoadDefVariables(obj, objVal, a_data);
                        }

                        field.SetValue(a_obj, obj);
                    }

                    break;
                }
                }
                
            }
            else
            {
                Logger.Error("FlareCS: Invalid Def Field: " + a_datObj.Name + ", " + a_data.Name + " : " + a_data.Path);
            }
        }

        static void LoadDefData(string a_path, ref List<DefData> a_data)
        {
            string[] files = Directory.GetFiles(a_path);

            foreach (string file in files)
            {
                FileInfo info = new FileInfo(file);

                if (info.Extension == ".def")
                {
                    XmlDocument doc = new XmlDocument();
                    doc.Load(file);

                    if (doc.DocumentElement is XmlElement root)
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
                                if (bool.TryParse(att.Value, out val))
                                {
                                    data.Abstract = val;
                                }
                                else
                                {
                                    Logger.Error("FlareCS: Error parsing Abstract value: " + att.Value + " : " + file);
                                }

                                break;
                            }
                            default:
                            {
                                Logger.Error("FlareCS: Invalid Def Attribute: " + att.Name + " : " + file);

                                break;
                            }
                            }
                        }

                        foreach (XmlNode node in root.ChildNodes)
                        {
                            if (node is XmlElement element)
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
                            Logger.Error("FlareCS: Error parsing unamed Def: " + file);
                        }
                    }
                }
            }

            string[] dirs = Directory.GetDirectories(a_path);
            foreach (string dir in dirs)
            {
                LoadDefData(dir, ref a_data);
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
                    Logger.Error("FlareCS: Cannot find def parent: " + a_data.Parent + ", " + a_data.Name + " : " + a_data.Path);

                    return false;
                }
            }

            a_def.DefName = a_data.Name;

            foreach (DefDataObject obj in a_data.DefDataObjects)
            {
                LoadDefVariables(a_def, obj, a_data);
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
                    Logger.Error("FlareCS: Error creating Def: " + a_data.Type + ", " + a_data.Name + " : " + a_data.Path);
                }
            }
            else
            {
                Logger.Error("FlareCS: Invalid Def Type: " + a_data.Type + ", " + a_data.Name + " : " + a_data.Path);
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
                Logger.Message("FlareCS: Loading Defs");

                List<DefData> defData = new List<DefData>();
                LoadDefData(a_path, ref defData);

                Logger.Message("FlareCS: Building DefTable");

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

        static void ResolveDefs(object a_obj)
        {
            Type type = a_obj.GetType();
            FieldInfo[] fieldInfo = type.GetFields(BindingFlags.Public | BindingFlags.Instance);
            foreach (FieldInfo info in fieldInfo)
            {
                Type fieldType = info.FieldType;

                if (fieldType.IsPrimitive || fieldType == typeof(string) || fieldType == typeof(decimal) || fieldType.IsSubclassOf(typeof(Enum)) || fieldType == typeof(Vector2) || fieldType == typeof(Vector3) || fieldType == typeof(Vector4))
                {
                    continue;
                }
                else if (fieldType.IsSubclassOf(typeof(Def)))
                {
                    Def stub = (Def)info.GetValue(a_obj);

                    if (stub != null)
                    {
                        Def resDef = GetDef(stub.DefName);

                        if (resDef != null)
                        {
                            info.SetValue(a_obj, resDef);
                        }
                        else
                        {
                            Logger.Error("FlareCS: Error resolving Def: " + stub.DefName);
                        }
                    }
                }
                else if (fieldType.IsGenericType && fieldType.GetGenericTypeDefinition() == typeof(List<>))
                {
                    object listObj = info.GetValue(a_obj);
                    if (listObj != null)
                    {
                        IEnumerable enumer = (IEnumerable)listObj;

                        Type genericType = fieldType.GetGenericArguments()[0];
                        if (genericType.IsSubclassOf(typeof(Def)))
                        {
                            MethodInfo methodInfo = fieldType.GetMethod("Add");
                            object list = Activator.CreateInstance(fieldType);

                            foreach (object obj in enumer)
                            {
                                if (obj is Def stub)
                                {
                                    methodInfo.Invoke(list, new object[] { GetDef(stub.DefName) });
                                }
                            }

                            info.SetValue(a_obj, list);
                        }
                        else
                        {
                            foreach (object obj in enumer)
                            {
                                ResolveDefs(obj);
                            }
                        }
                    }
                }
                else
                {
                    ResolveDefs(info.GetValue(a_obj));
                }
            }
        }

        public static void ResolveDefs()
        {
            foreach (Def def in m_defs)
            {
                ResolveDefs(def);
            }

            foreach (Def def in m_defs)
            {
                def.PostResolve();
            }
        }

        public static List<T> GetDefs<T>() where T : Def
        {
            List<T> defs = new List<T>();
            
            foreach (Def def in m_defs)
            {
                if (def is T t)
                {
                    defs.Add(t);
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

            Logger.Warning($"FlareCS: Cannot find def of name: {a_name}");

            return null;
        }
        public static T GetDef<T>(string a_name) where T : Def
        {
            if (m_defLookup.ContainsKey(a_name))
            {
                Def def = m_defLookup[a_name];
                if (def is T t)
                {
                    return t;
                }
            }

            Logger.Warning($"FlareCS: Cannot find def of name and type: {a_name}, {typeof(T).ToString()}");

            return null;
        }
    };
}