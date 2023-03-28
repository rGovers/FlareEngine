#include "Rendering/IO/OBJLoader.h"

#include <fstream>
#include <string.h>
#include <unordered_map>

#include "FlareAssert.h"
#include "Logger.h"
#include "Trace.h"

enum e_FaceMode
{
    FaceMode_Position,
    FaceMode_PositionNormal,
    FaceMode_PositionTexCoords,
    FaceMode_PositionNormalTexcoords
};

struct IndexMap
{
    uint32_t PositionIndex;
    uint32_t NormalIndex;
    uint32_t TexCoordsIndex;
};

constexpr std::string_view VertexPositionStr = "v";
constexpr std::string_view VertexNormalStr = "vn";
constexpr std::string_view VertexTexCoordsStr = "vt";
constexpr std::string_view IndicesFaceStr = "f";

bool OBJLoader_LoadFile(const std::filesystem::path& a_path, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices)
{
    if (std::filesystem::exists(a_path))
    {
        TRACE("Loading OBJ Model");
        std::ifstream file = std::ifstream(a_path);

        if (file.good() && file.is_open())
        {
            file.ignore(std::numeric_limits<std::streamsize>::max());
            const std::streamsize size = file.gcount();
            file.clear();
            file.seekg(0, std::ios::beg);

            char* dat = new char[size];
            file.read(dat, size);

            std::vector<glm::vec4> positions;
            std::vector<glm::vec3> normals;
            std::vector<glm::vec2> texcoords;

            std::vector<IndexMap> indices;

            const char* s = dat;
            while (s - dat < size)
            {
                while (*s == '\n' || *s == ' ')
                {
                    ++s;
                    
                    if ((s + 1) - dat >= size)
                    {
                        goto Exit;
                    }
                }

                const char* nl = s + 1;
                while (*nl != '\n')
                {
                    ++nl;

                    if (nl - dat >= size)
                    {
                        goto Exit;
                    }
                }

                const char* spc = s + 1;
                while (*spc != ' ')
                {
                    ++spc;
                }
                
                const uint32_t len = spc - s;

                if (len == VertexPositionStr.size() && strncmp(s, VertexPositionStr.data(), VertexPositionStr.size()) == 0)
                {
                    const char* xStr = spc + 1;
                    while (*xStr == ' ')
                    {
                        ++xStr;
                    }

                    const char* yStr = xStr + 1;
                    while (*yStr != ' ')
                    {
                        ++yStr;
                    }
                    while (*yStr == ' ')
                    {
                        ++yStr;
                    }
                    
                    const char* zStr = yStr + 1;
                    while (*zStr != ' ')
                    {
                        ++zStr;
                    }
                    while (*zStr == ' ')
                    {
                        ++zStr;
                    }

                    const char* next = zStr + 1;
                    while (*next != '\n' && *next != ' ')
                    {
                        ++next;
                    }
                    
                    const char* wStr = nullptr;
                    if (*next != '\n')
                    {
                        wStr = next;
                        while (*wStr == ' ')
                        {
                            ++wStr;
                        }

                        if (*wStr == '\n')
                        {
                            wStr = nullptr;
                        }
                    }
                    
                    glm::vec4 pos;

                    pos.x = std::stof(std::string(xStr, yStr - xStr));
                    pos.y = std::stof(std::string(yStr, zStr - yStr));

                    // W component is optional in file spec
                    if (wStr != nullptr)
                    {
                        pos.z = std::stof(std::string(zStr, wStr - zStr));
                        pos.w = std::stof(std::string(wStr, nl - wStr));
                    }
                    else
                    {
                        pos.z = std::stof(std::string(zStr, nl - zStr));

                        pos.w = 1.0f;
                    }

                    positions.emplace_back(pos);
                }
                else if (len == VertexNormalStr.size() && strncmp(s, VertexNormalStr.data(), VertexNormalStr.size()) == 0)
                {
                    const char* xStr = spc + 1;
                    while (*xStr == ' ')
                    {
                        ++xStr;
                    }

                    const char* yStr = xStr + 1;
                    while (*yStr != ' ')
                    {
                        ++yStr;
                    }
                    while (*yStr == ' ')
                    {
                        ++yStr;
                    }

                    const char* zStr = yStr + 1;
                    while (*zStr != ' ')
                    {
                        ++zStr;
                    }
                    while (*zStr == ' ')
                    {
                        ++zStr;
                    }

                    glm::vec3 norm;

                    norm.x = std::stof(std::string(xStr, yStr - xStr));
                    norm.y = std::stof(std::string(yStr, zStr - yStr));
                    norm.z = std::stof(std::string(zStr, nl - zStr));

                    // File spec does not guarantee normalization
                    normals.emplace_back(glm::normalize(norm));
                }
                else if (len == VertexTexCoordsStr.size() && strncmp(s, VertexTexCoordsStr.data(), VertexTexCoordsStr.size()) == 0)
                {
                    const char* uStr = spc + 1;
                    while (*uStr == ' ')
                    {
                        ++uStr;
                    }

                    const char* next = uStr + 1;
                    while (*next != '\n' && *next != ' ')
                    {
                        ++next;
                    }

                    // V component is optional
                    const char* vStr = nullptr;
                    if (*next != '\n')
                    {
                        vStr = next;
                        while (*vStr == ' ')
                        {
                            ++vStr;
                        }

                        if (*vStr == '\n')
                        {
                            vStr = nullptr;
                        }
                    }

                    glm::vec2 uv;
                    if (vStr != nullptr)
                    {
                        uv.x = std::stof(std::string(uStr, vStr - uStr));

                        // Can be a W component but dont care about it
                        next = vStr + 1;
                        while (*next != '\n' && *next != ' ')
                        {
                            ++next;
                        }

                        uv.y = std::stof(std::string(vStr, next - vStr));
                    }
                    else
                    {
                        uv.x = std::stof(std::string(uStr, nl - uStr));
                        uv.y = 0.0f;
                    }

                    texcoords.emplace_back(uv);
                }
                else if (len == IndicesFaceStr.size() && strncmp(s, IndicesFaceStr.data(), IndicesFaceStr.size()) == 0)
                {
                    const char* iA = spc + 1;
                    while (*iA == ' ')
                    {
                        ++iA;
                    }

                    const char* sl[2];
                    sl[0] = nullptr;
                    sl[1] = nullptr;

                    const char* s = iA + 1;
                    while (*s != ' ')
                    {
                        if (*s == '/')
                        {
                            if (sl[0] == nullptr)
                            {
                                sl[0] = s;
                            }
                            else
                            {
                                sl[1] = s;

                                break;
                            }
                        }

                        ++s;
                    } 

                    e_FaceMode faceMode;
                    if (sl[0] != nullptr && sl[1] != nullptr)
                    {
                        if (sl[0] + 1 == sl[1])
                        {
                            faceMode = FaceMode_PositionNormal;
                        }
                        else
                        {
                            faceMode = FaceMode_PositionNormalTexcoords;
                        }
                    }
                    else if (sl[0] != nullptr)
                    {
                        faceMode = FaceMode_PositionTexCoords;
                    }
                    else
                    {
                        faceMode = FaceMode_Position;
                    }

                    const char* iB = iA + 1;
                    while (*iB != ' ')
                    {
                        ++iB;
                    }
                    while (*iB == ' ')
                    {
                        ++iB;
                    }
                    
                    const char* iC = iB + 1;
                    while (*iC != ' ')
                    {
                        ++iC;
                    }
                    while (*iC == ' ')
                    {
                        ++iC;
                    }

                    const char* next = iC + 1;
                    while (*next != ' ' && *next != '\n')
                    {
                        ++next;
                    }

                    const char* iD = nullptr;
                    if (*next != '\n')
                    {
                        iD = next + 1;
                        while (*iD == ' ')
                        {
                            ++iD;
                        }

                        if (*iD == '\n')
                        {
                            iD = nullptr;
                        }
                    }

                    const char* indMap[5] = { iA, iB, iC, iD, nl };
                    const bool quad = iD != nullptr;

                    int indexCount = 4;
                    if (!quad)
                    {
                        indexCount = 3;
                        indMap[3] = nl;
                        indMap[4] = nullptr;
                    }

                    IndexMap tIndices[4];

                    switch (faceMode)
                    {
                    case FaceMode_Position:
                    {
                        for (int i = 0; i < indexCount; ++i)
                        {
                            const char* cur = indMap[i];
                            const char* next = indMap[i + 1];

                            tIndices[i].PositionIndex = (uint32_t)std::stoll(std::string(cur, next - cur));
                            tIndices[i].NormalIndex = -1;
                            tIndices[i].TexCoordsIndex = -1;
                        }

                        break;
                    }
                    case FaceMode_PositionTexCoords:
                    {
                        for (int i = 0; i < indexCount; ++i)
                        {
                            const char* cur = indMap[i];
                            const char* next = indMap[i + 1];

                            const char* s = cur + 1;
                            while (*s != '/')
                            {
                                ++s;
                            }

                            tIndices[i].PositionIndex = (uint32_t)std::stoll(std::string(cur, s - cur));
                            tIndices[i].NormalIndex = -1;
                            tIndices[i].TexCoordsIndex = (uint32_t)std::stoll(std::string(s + 1, next - s - 1));
                        }

                        break;
                    }
                    case FaceMode_PositionNormalTexcoords:
                    {
                        for (int i = 0; i < indexCount; ++i)
                        {
                            const char* cur = indMap[i];
                            const char* next = indMap[i + 1];

                            const char* sA = cur + 1;
                            while (*sA != '/')
                            {
                                ++sA;
                            }

                            const char* sB = sA + 1;
                            while (*sB != '/')
                            {
                                ++sB;
                            }

                            tIndices[i].PositionIndex = (uint32_t)std::stoll(std::string(cur, sA - cur));
                            tIndices[i].NormalIndex = (uint32_t)std::stoll(std::string(sB + 1, (next - sB) - 1));
                            tIndices[i].TexCoordsIndex = (uint32_t)std::stoll(std::string(sA + 1, (sB - sA) - 1));
                        }

                        break;
                    }
                    case FaceMode_PositionNormal:
                    {
                        for (int i = 0; i < indexCount; ++i)
                        {
                            const char* cur = indMap[i];
                            const char* next = indMap[i + 1];

                            const char* s = cur + 1;
                            while (*s != '/')
                            {
                                ++s;
                            }

                            tIndices[i].PositionIndex = (uint32_t)std::stoll(std::string(cur, s - cur));
                            tIndices[i].NormalIndex = (uint32_t)std::stoll(std::string(s + 2, next - s - 2));
                            tIndices[i].TexCoordsIndex = -1;
                        }

                        break;
                    }
                    default:
                    {   
                        Logger::Warning("OBJLoader: Invalid face mode");

                        break;
                    }
                    }


                    indices.emplace_back(tIndices[0]);
                    indices.emplace_back(tIndices[1]);
                    indices.emplace_back(tIndices[2]);

                    if (quad)
                    {
                        indices.emplace_back(tIndices[0]);
                        indices.emplace_back(tIndices[2]);
                        indices.emplace_back(tIndices[3]);
                    }
                }

                s = nl + 1;
            }
            
Exit:;
            delete[] dat;

            std::unordered_map<uint64_t, uint32_t> indexMap;

            for (const IndexMap& iMap : indices)
            {
                // https://en.wikipedia.org/wiki/Pairing_function#Cantor_pairing_function
                const uint64_t abH = ((uint64_t)iMap.PositionIndex + iMap.NormalIndex) * ((uint64_t)iMap.PositionIndex + iMap.NormalIndex + 1) / 2 + iMap.NormalIndex;
                const uint64_t h = ((abH + iMap.TexCoordsIndex) * (abH + iMap.TexCoordsIndex + 1) / 2 + iMap.TexCoordsIndex);
                
                const auto iter = indexMap.find(h);
                if (iter != indexMap.end())
                {
                    a_indices->emplace_back(iter->second);
                }
                else
                {
                    const uint32_t index = (uint32_t)a_vertices->size();
                    a_indices->emplace_back(index);

                    indexMap.emplace(h, index);

                    Vertex v;

                    FLARE_ASSERT(iMap.PositionIndex - 1 < positions.size());
                    v.Position = positions[iMap.PositionIndex - 1];
                    
                    if (iMap.NormalIndex != -1)
                    {
                        FLARE_ASSERT(iMap.NormalIndex - 1 < normals.size());
                        v.Normal = normals[iMap.NormalIndex - 1];
                    }

                    if (iMap.TexCoordsIndex != -1)
                    {
                        FLARE_ASSERT(iMap.TexCoordsIndex - 1 < texcoords.size());
                        v.TexCoords = texcoords[iMap.TexCoordsIndex - 1];
                    }

                    v.Color = glm::vec4(1.0f);

                    a_vertices->emplace_back(v);
                }
            }

            return true;
        }
    }

    return false;
}