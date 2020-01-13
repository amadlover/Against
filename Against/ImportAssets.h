#pragma once

#include "Asset.h"

int ImportGLTF (const char* FilePath, Node** Nodes, uint32_t* NodeCount, Mesh** Meshes, uint32_t* MeshCount, Material_Orig** Materials, uint32_t* MaterialCount, Texture_Orig** Textures, uint32_t* TextureCount, Image_Orig** Images, uint32_t* ImageCount, Sampler** Samplers, uint32_t* SamplerCount);
