#pragma once

#include <filesystem>
#include <vector>

#include "Rendering/Vertices.h"

bool OBJLoader_LoadFile(const std::filesystem::path& a_path, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices);