#pragma once

#include "Asset.h"

int ImportGLTF (const char* FilePath, Node** Nodes, uint32_t* NodeCount, Mesh** Meshes, uint32_t* MeshCount, Material** Materials, uint32_t* MaterialCount, Texture** Textures, uint32_t* TextureCount, Image** Images, uint32_t* ImageCount, Sampler** Samplers, uint32_t* SamplerCount);

int ImportAssets (const char* FilePath, Asset** Assets, uint32_t* AssetCount);