#pragma once

#include "Asset.h"

int ImportGLTF (const char* FilePath, Node_Orig** Nodes, uint32_t* NodeCount, Mesh_Orig** Meshes, uint32_t* MeshCount, Material_Orig** Materials, uint32_t* MaterialCount, Texture_Orig** Textures, uint32_t* TextureCount, Image_Orig** Images, uint32_t* ImageCount, Sampler_Orig** Samplers, uint32_t* SamplerCount);
