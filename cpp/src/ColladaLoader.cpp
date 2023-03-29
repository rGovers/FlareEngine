#include "Rendering/IO/ColladaLoader.h"

#include <tinyxml2.h>

#include "FlareAssert.h"
#include "Trace.h"

enum e_ColladaUpAxis
{
    ColladaUpAxis_XUp,
    ColladaUpAxis_YUp,
    ColladaUpAxis_ZUp
};

enum e_ColladaSourceDataType
{
    ColladaSourceDataType_Null = -1,
    ColladaSourceDataType_Float
};

struct ColladaParam
{
    std::string Name;
    e_ColladaSourceDataType Type;
};
struct ColladaInput
{
    std::string Semantic;
    std::string Source;
    uint32_t Offset;
};

struct ColladaAccessor
{
    std::string Source;
    uint32_t Stride;
    uint32_t Offset;
    uint32_t Count;
    std::vector<ColladaParam> Params;
};
struct ColladaData
{
    std::string ID;
    void* Data;
    e_ColladaSourceDataType Type;
};

struct ColladaSource
{
    std::string ID;
    std::string Name;
    ColladaData Data;
    ColladaAccessor Accessor;
};

struct ColladaPolylist
{
    std::vector<ColladaInput> Inputs;
    std::vector<uint32_t> VCount;
    std::vector<uint32_t> P;
};

struct ColladaMesh
{
    std::vector<ColladaSource> Sources;
    std::vector<ColladaInput> Vertices;
    ColladaPolylist Polylist;
};

struct ColladaGeometry
{
    std::string ID;
    std::string Name;
    ColladaMesh Mesh;
};

ColladaMesh LoadMesh(const tinyxml2::XMLElement* a_meshElement)
{
    ColladaMesh mesh;

    for (const tinyxml2::XMLElement* sourceElement = a_meshElement->FirstChildElement(); sourceElement != nullptr; sourceElement = sourceElement->NextSiblingElement())
    {
        const char* name = sourceElement->Value();
        
        if (strcmp(name, "source") == 0)
        {
            ColladaSource s;
            s.ID = std::string(sourceElement->Attribute("id"));
            s.Name = std::string(sourceElement->Attribute("name"));
        
            for (const tinyxml2::XMLElement* sDataElement = sourceElement->FirstChildElement(); sDataElement != nullptr; sDataElement = sDataElement->NextSiblingElement())
            {
                const char* name = sDataElement->Value();

                if (strcmp(name, "float_array") == 0)
                {
                    ColladaData d;
                    d.ID = std::string(sDataElement->Attribute("id"));
                    d.Type = ColladaSourceDataType_Float;

                    const uint32_t count = (uint32_t)sDataElement->Int64Attribute("count");

                    const char* data = sDataElement->GetText();

                    d.Data = new float[count];
                    float* fDat = (float*)d.Data;

                    const char* sC = data;
                    uint32_t index = 0;

                    while (*sC != 0 && index < count)
                    {
                        while (*sC == ' ' || *sC == '\n')
                        {
                            ++sC;
                        }

                        if (*sC == 0)
                        {
                            break;
                        }

                        const char* next = sC + 1;
                        while (*next != ' ' && *next != 0)
                        {
                            ++next;
                        }

                        fDat[index++] = std::stof(std::string(sC, next - sC));

                        sC = next;
                    }

                    s.Data = d;
                }
                else if (strcmp(name, "technique_common") == 0)
                {
                    const tinyxml2::XMLElement* accessorElement = sDataElement->FirstChildElement("accessor");
                    FLARE_ASSERT(accessorElement != nullptr);

                    ColladaAccessor a;
                    a.Source = std::string(accessorElement->Attribute("source"));
                    a.Count = (uint32_t)accessorElement->Int64Attribute("count");
                    a.Stride = (uint32_t)accessorElement->IntAttribute("stride");
                    a.Count = (uint32_t)accessorElement->IntAttribute("offset");

                    for (const tinyxml2::XMLElement* paramElement = accessorElement->FirstChildElement(); paramElement != nullptr; paramElement = paramElement->NextSiblingElement())
                    {
                        const char* name = paramElement->Value();
                        if (strcmp(name, "param") == 0)
                        {
                            ColladaParam p;
                            p.Name = std::string(paramElement->Attribute("name"));
                            p.Type = ColladaSourceDataType_Null;

                            const char* type = paramElement->Attribute("type");
                            if (strcmp(type, "float") == 0)
                            {
                                p.Type = ColladaSourceDataType_Float;
                            }

                            a.Params.emplace_back(p);
                        }
                    }

                    s.Accessor = a;
                }
            }

            mesh.Sources.emplace_back(s);
        }
        else if (strcmp(name, "vertices") == 0)
        {
            for (const tinyxml2::XMLElement* inputElement = sourceElement->FirstChildElement(); inputElement != nullptr; inputElement = inputElement->NextSiblingElement())
            {
                ColladaInput i;
                i.Semantic = std::string(inputElement->Attribute("semantic"));
                i.Source = std::string(inputElement->Attribute("source"));
                i.Offset = (uint32_t)inputElement->IntAttribute("offset");

                mesh.Vertices.emplace_back(i);
            }
        }
        else if (strcmp(name, "polylist") == 0)
        {
            ColladaPolylist p;

            for (const tinyxml2::XMLElement* element = sourceElement->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
            {
                const char* name = element->Value();
                if (strcmp(name, "input") == 0)
                {
                    ColladaInput i;
                    i.Semantic = std::string(element->Attribute("semantic"));
                    i.Source = std::string(element->Attribute("source"));
                    i.Offset = (uint32_t)element->IntAttribute("offset");

                    p.Inputs.emplace_back(i);
                }
                else if (strcmp(name, "vcount") == 0)
                {
                    const char* data = element->GetText();
                    const char* s = data;
                    while (*s != 0)
                    {
                        while (*s == ' ')
                        {
                            ++s;
                        }

                        if (*s == 0)
                        {
                            break;
                        }

                        const char* next = s + 1;
                        while (*next != ' ' && *next != 0)
                        {
                            ++next;
                        }

                        p.VCount.emplace_back((uint32_t)std::stoi(std::string(s, next - s)));

                        s = next;
                    }
                }
                else if (strcmp(name, "p") == 0)
                {
                    const char* data = element->GetText();
                    const char* s = data;
                    while (*s != 0)
                    {
                        while (*s == ' ')
                        {
                            ++s;
                        }

                        if (*s == 0)
                        {
                            break;
                        }

                        const char* next = s + 1;
                        while (*next != ' ' && *next != 0)
                        {
                            ++next;
                        }

                        p.P.emplace_back((uint32_t)std::stoll(std::string(s, next - s)));

                        s = next;
                    }
                }
            }

            mesh.Polylist = p;
        }
    }

    return mesh;
}

std::vector<ColladaGeometry> LoadGeometry(const tinyxml2::XMLElement* a_libraryElement)
{
    std::vector<ColladaGeometry> geometryLib;

    for (const tinyxml2::XMLElement* geomElement = a_libraryElement->FirstChildElement(); geomElement != nullptr; geomElement = geomElement->NextSiblingElement())
    {
        if (strcmp(geomElement->Value(), "geometry") == 0)
        {
            ColladaGeometry g;
            g.ID = std::string(geomElement->Attribute("id"));
            g.Name = std::string(geomElement->Attribute("name"));

            const tinyxml2::XMLElement* meshElement = geomElement->FirstChildElement("mesh");
            FLARE_ASSERT(meshElement != nullptr);

            g.Mesh = LoadMesh(meshElement);

            geometryLib.emplace_back(g);
        }
    }

    return geometryLib;
}

int* ColladaLoader_GetOffsets(const ColladaSource& a_source, int* a_count)
{
    *a_count = (int)a_source.Accessor.Params.size();

    int* offset = new int[*a_count];
    for (uint32_t i = 0; i < *a_count; ++i)
    {
        offset[i] = 0;
        
        const ColladaParam& p = a_source.Accessor.Params[i];
        if (p.Name == "X" || p.Name == "S")
        {
            offset[i] = 0;
        }
        else if (p.Name == "Y" || p.Name == "T")
        {
            offset[i] = 1;
        }
        else if (p.Name == "Z")
        {
            offset[i] = 2;
        }
        else if (p.Name == "W")
        {
            offset[i] = 3;
        }
    }

    return offset;
}

bool ColladaLoader_LoadFile(const std::filesystem::path& a_path, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices)
{
    if (std::filesystem::exists(a_path))
    {
        TRACE("Loading Collada Model");
        tinyxml2::XMLDocument doc = tinyxml2::XMLDocument();
        if (doc.LoadFile(a_path.c_str()) == tinyxml2::XML_SUCCESS)
        {
            const tinyxml2::XMLElement* rootElement = doc.RootElement();

            e_ColladaUpAxis up = ColladaUpAxis_YUp;
            float scale = 1.0f;
            std::vector<ColladaGeometry> geometry;

            for (const tinyxml2::XMLElement* element = rootElement->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
            {
                const char* elementName = element->Value();

                if (strcmp(elementName, "asset") == 0)
                {
                    for (const tinyxml2::XMLElement* assetElement = element->FirstChildElement(); assetElement != nullptr; assetElement = assetElement->NextSiblingElement())
                    {
                        const char* name = assetElement->Value();
                        if (strcmp(name, "up_axis") == 0)
                        {
                            const char* value = assetElement->GetText();
                            if (strcmp(value, "X_UP") == 0)
                            {
                                up = ColladaUpAxis_XUp;
                            }
                            else if (strcmp(value, "Y_UP") == 0)
                            {
                                up = ColladaUpAxis_YUp;
                            }
                            else if (strcmp(value, "Z_UP") == 0)
                            {
                                up = ColladaUpAxis_ZUp;
                            }
                        }
                        else if (strcmp(name, "unit") == 0)
                        {
                            scale = assetElement->FloatAttribute("meter");
                        }
                    }
                }
                else if (strcmp(elementName, "library_geometries") == 0)
                {
                    geometry = LoadGeometry(element);
                }
            }

            for (const ColladaGeometry& g : geometry)
            {
                ColladaInput posInput;
                ColladaInput normalInput;
                ColladaInput texcoordInput;

                for (const ColladaInput& pI : g.Mesh.Polylist.Inputs)
                {
                    if (pI.Semantic == "POSITION")
                    {
                        posInput = pI;
                    }
                    else if (pI.Semantic == "NORMAL")
                    {
                        normalInput = pI;
                    }
                    else if (pI.Semantic == "TEXCOORD")
                    {
                        texcoordInput = pI;
                    }
                    else if (pI.Semantic == "VERTEX")
                    {
                        for (const ColladaInput& vI : g.Mesh.Vertices)
                        {
                            if (vI.Semantic == "POSITION")
                            {
                                posInput = vI;
                                posInput.Offset = vI.Offset;
                            }
                            else if (vI.Semantic == "NORMAL")
                            {
                                normalInput = vI;
                                normalInput.Offset = vI.Offset;
                            }
                            else if (vI.Semantic == "TEXCOORD")
                            {
                                texcoordInput = vI;
                                texcoordInput.Offset = vI.Offset;
                            }
                        }
                    }
                }

                ColladaSource posSource;
                ColladaSource normalSource;
                ColladaSource texcoordSource;

                for (const ColladaSource& d : g.Mesh.Sources)
                {
                    const std::string idStr = "#" + d.ID;
                    if (idStr == posInput.Source)
                    {
                        posSource = d;
                    }
                    else if (idStr == normalInput.Source)
                    {
                        normalSource = d;
                    }
                    else if (idStr == texcoordInput.Source)
                    {
                        texcoordSource = d;
                    }
                }

                uint32_t index = 0;
                const uint32_t count = (uint32_t)g.Mesh.Polylist.Inputs.size();

                int posVCount;
                int normVCount;
                int texVCount;

                int* posOffset = ColladaLoader_GetOffsets(posSource, &posVCount);
                int* normalOffset = ColladaLoader_GetOffsets(normalSource, &normVCount);
                int* texcoordOffset = ColladaLoader_GetOffsets(texcoordSource, &texVCount);

                std::unordered_map<uint64_t, uint32_t> indexMap;

                for (uint32_t vCount : g.Mesh.Polylist.VCount)
                {
                    uint32_t posIndices[4];
                    uint32_t normalIndices[4];
                    uint32_t texcoordIndices[4];

                    for (int i = 0; i < vCount; ++i)
                    {
                        // Flip faces so back culling work correctly
                        const uint32_t iIndex = index + ((vCount - 1) - i) * count;

                        posIndices[i] = g.Mesh.Polylist.P[iIndex + posInput.Offset];
                        normalIndices[i] = g.Mesh.Polylist.P[iIndex + normalInput.Offset];
                        texcoordIndices[i] = g.Mesh.Polylist.P[iIndex + texcoordInput.Offset];
                    }

                    switch (vCount)
                    {
                    case 3:
                    {
                        for (int i = 0; i < 3; ++i)
                        {
                            // https://en.wikipedia.org/wiki/Pairing_function#Cantor_pairing_function
                            const uint64_t abH = ((uint64_t)posIndices[i] + normalIndices[i]) * ((uint64_t)posIndices[i] + normalIndices[i] + 1) / 2 + normalIndices[i];
                            const uint64_t h = (abH + texcoordIndices[i]) * (abH + texcoordIndices[i] + 1) / 2 + texcoordIndices[i];

                            const auto iter = indexMap.find(h);
                            if (iter != indexMap.end())
                            {
                                a_indices->emplace_back(iter->second);
                            }
                            else
                            {
                                const uint32_t vIndex = (uint32_t)a_vertices->size();

                                a_indices->emplace_back(vIndex);

                                Vertex v = Vertex();
                                for (int j = 0; j < posVCount; ++j)
                                {
                                    if (posOffset[j] == 1)
                                    {
                                        v.Position[posOffset[j]] = -((float*)posSource.Data.Data)[(posIndices[i] * posSource.Accessor.Stride) + j] * scale;
                                    }
                                    else
                                    {
                                        v.Position[posOffset[j]] = ((float*)posSource.Data.Data)[(posIndices[i] * posSource.Accessor.Stride) + j] * scale;
                                    }
                                }

                                for (int j = 0; j < normVCount; ++j)
                                {
                                    if (normalOffset[j] == 1)
                                    {
                                        v.Normal[normalOffset[j]] = -((float*)normalSource.Data.Data)[(normalIndices[i] * normalSource.Accessor.Stride) + j];
                                    }
                                    else
                                    {
                                        v.Normal[normalOffset[j]] = ((float*)normalSource.Data.Data)[(normalIndices[i] * normalSource.Accessor.Stride) + j];
                                    }
                                }

                                for (int j = 0; j < texVCount; ++j)
                                {
                                    v.TexCoords[texcoordOffset[j]] = ((float*)texcoordSource.Data.Data)[(texcoordIndices[i] * texcoordSource.Accessor.Stride) + j];
                                }

                                a_vertices->emplace_back(v);

                                indexMap.emplace(h, vIndex);
                            }
                        }

                        break;
                    }
                    case 4:
                    {
                        // TODO: Implement quads

                        break;
                    }
                    }

                    index += count * vCount;
                }

                delete[] posOffset;
                delete[] normalOffset;
                delete[] texcoordOffset;
            }

            for (const ColladaGeometry& g : geometry)
            {
                for (const ColladaSource& s : g.Mesh.Sources)
                {
                    switch (s.Data.Type)
                    {
                    case ColladaSourceDataType_Float:
                    {
                        delete[] (float*)s.Data.Data;

                        break;
                    }
                    }
                }
            }

            return true;
        }  
    }

    return false;
}