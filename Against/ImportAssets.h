#pragma once

#include "Assets.h"
#include <cgltf.h>

cgltf_data* ReturnGLTF (const char* Filename);
int ImportMainMenuGLTF (const char* Filename, Mesh* Meshes, uint32_t* MeshCount);