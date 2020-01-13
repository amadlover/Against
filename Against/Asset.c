#include "Asset.h"
#include "Error.h"

#include "GraphicsUtilities.h"
#include "Utility.h"

#include <Windows.h>
#include <Shlwapi.h>
#include <strsafe.h>

#include <string.h>

#include <cgltf.h>
#include <stb_image.h>

typedef enum
{
	GraphicsPrimitiveType,
	PhysicsPrimitiveType
} EPrimitiveType;

int GetFullTexturePath (const char* FilePath, const char* URI, char* FullFilePath)
{
	TCHAR TextureFile[MAX_PATH];
	mbstowcs (TextureFile, FilePath, MAX_PATH);

	PathRemoveFileSpec (TextureFile);
	TCHAR Uri[MAX_PATH];
	mbstowcs (Uri, URI, MAX_PATH);
	
	TCHAR URIPath[MAX_PATH];
	StringCchCopy (URIPath, MAX_PATH, L"\\");
	StringCchCat (URIPath, MAX_PATH, Uri);

	StringCchCat (TextureFile, MAX_PATH, URIPath);
	wcstombs (FullFilePath, TextureFile, MAX_PATH);

	return 0;
}

void ImportAttribute (PrimitivePtr* PrimPtr, const cgltf_attribute* Attribute, EPrimitiveType PrimType)
{
	cgltf_accessor* Accessor = Attribute->data;
	cgltf_buffer_view* BufferView = Accessor->buffer_view;

	if (Attribute->type == cgltf_attribute_type_position)
	{
		char* DataStart = (char*)BufferView->buffer->data;
		float* Positions = (float*)(DataStart + Accessor->offset + BufferView->offset);

		if (PrimType == GraphicsPrimitiveType)
		{
			PrimPtr->GraphicsPrimPtr->PositionSize = BufferView->size;
			PrimPtr->GraphicsPrimPtr->Positions = (float*)MyMalloc (BufferView->size);
			memcpy (PrimPtr->GraphicsPrimPtr->Positions, Positions, BufferView->size);
		}
		else if (PrimType == PhysicsPrimitiveType)
		{
			PrimPtr->PhysicsPrimPtr->PositionSize = BufferView->size;
			PrimPtr->PhysicsPrimPtr->Positions = (float*)MyMalloc (BufferView->size);
			memcpy (PrimPtr->PhysicsPrimPtr->Positions, Positions, BufferView->size);
		}

	}
	else if (Attribute->type == cgltf_attribute_type_texcoord)
	{
		if (strcmp (Attribute->name, "TEXCOORD_0") == 0)
		{
			char* DataStart = (char*)BufferView->buffer->data;
			float* UVs = (float*)(DataStart + Accessor->offset + BufferView->offset);

			if (PrimType == GraphicsPrimitiveType)
			{
				PrimPtr->GraphicsPrimPtr->UV0Size = BufferView->size;
				PrimPtr->GraphicsPrimPtr->UV0s = (float*)MyMalloc (BufferView->size);
				memcpy (PrimPtr->GraphicsPrimPtr->UV0s, UVs, BufferView->size);
			}
		}
	}
}

void ImportMaterials (const char* FilePath, PrimitivePtr* PrimPtr, const cgltf_material* Material)
{
	strcpy (PrimPtr->GraphicsPrimPtr->Material.Name, Material->name);

	if (Material->has_pbr_metallic_roughness)
	{
		if (Material->pbr_metallic_roughness.base_color_texture.texture->name != NULL)
		{
			strcpy (PrimPtr->GraphicsPrimPtr->Material.BaseColorTexture.Name, Material->pbr_metallic_roughness.base_color_texture.texture->name);
		}

		strcpy (PrimPtr->GraphicsPrimPtr->Material.BaseColorTexture.Image.Name, Material->pbr_metallic_roughness.base_color_texture.texture->image->name);

		char FullPath[256];
		GetFullTexturePath (FilePath, Material->pbr_metallic_roughness.base_color_texture.texture->image->uri, FullPath);

		PrimPtr->GraphicsPrimPtr->Material.BaseColorTexture.Image.Pixels = stbi_load (FullPath, (int*)&PrimPtr->GraphicsPrimPtr->Material.BaseColorTexture.Image.Width, (int*)&PrimPtr->GraphicsPrimPtr->Material.BaseColorTexture.Image.Height, (int*)&PrimPtr->GraphicsPrimPtr->Material.BaseColorTexture.Image.BPP, 4);
		PrimPtr->GraphicsPrimPtr->Material.BaseColorTexture.Image.Size = PrimPtr->GraphicsPrimPtr->Material.BaseColorTexture.Image.Width * PrimPtr->GraphicsPrimPtr->Material.BaseColorTexture.Image.Height * PrimPtr->GraphicsPrimPtr->Material.BaseColorTexture.Image.BPP;
	}
}

void ImportIndices (PrimitivePtr* PrimPtr, const cgltf_primitive* Primitive, EPrimitiveType PrimType)
{
	cgltf_accessor* Accessor = Primitive->indices;

	if (PrimType == GraphicsPrimitiveType)
	{
		PrimPtr->GraphicsPrimPtr->IndexCount = Accessor->count;
	}
	else if (PrimType = PhysicsPrimitiveType)
	{
		PrimPtr->PhysicsPrimPtr->IndexCount = Accessor->count;
	}

	cgltf_buffer_view* BufferView = Accessor->buffer_view;

	char* DataStart = (char*)BufferView->buffer->data;

	switch (Accessor->component_type)
	{
	case cgltf_component_type_r_16u:
		if (PrimType == GraphicsPrimitiveType)
		{
			PrimPtr->GraphicsPrimPtr->IndexSize = BufferView->size * 2;
			PrimPtr->GraphicsPrimPtr->Indices = (uint32_t*)MyMalloc (2 * BufferView->size);
		}
		else if (PrimType == PhysicsPrimitiveType)
		{
			PrimPtr->PhysicsPrimPtr->IndexSize = BufferView->size * 2;
			PrimPtr->PhysicsPrimPtr->Indices = (uint32_t*)MyMalloc (2 * BufferView->size);
		}

		uint16_t* I16 = (uint16_t*)(DataStart + Accessor->offset + BufferView->offset);

		for (uint32_t i = 0; i < Accessor->count; i++)
		{
			if (PrimType == GraphicsPrimitiveType)
			{
				PrimPtr->GraphicsPrimPtr->Indices[i] = I16[i];
			}
			else if (PrimType == PhysicsPrimitiveType)
			{
				PrimPtr->PhysicsPrimPtr->Indices[i] = I16[i];
			}
		}

		break;

	case cgltf_component_type_r_32u:
		if (PrimType == GraphicsPrimitiveType)
		{
			PrimPtr->GraphicsPrimPtr->IndexSize = BufferView->size;
			PrimPtr->GraphicsPrimPtr->Indices = (uint32_t*)MyMalloc (BufferView->size);
		}
		else if (PrimType == PhysicsPrimitiveType)
		{
			PrimPtr->PhysicsPrimPtr->IndexSize = BufferView->size;
			PrimPtr->PhysicsPrimPtr->Indices = (uint32_t*)MyMalloc (BufferView->size);
		}

		uint32_t* I32 = (uint32_t*)(DataStart + Accessor->offset + BufferView->offset);

		if (PrimType == GraphicsPrimitiveType)
		{
			memcpy (PrimPtr->GraphicsPrimPtr->Indices, I32, BufferView->size);
		}
		else if (PrimType == PhysicsPrimitiveType)
		{
			memcpy (PrimPtr->PhysicsPrimPtr->Indices, I32, BufferView->size);
		}

		break;

	default:
		break;
	}
}

void ImportGraphicsPrimitives (const char* FilePath, Asset* Assets, cgltf_data* Data)
{
	uint32_t CurrentMeshAssetCount = 0;

	for (uint32_t n = 0; n < Data->nodes_count; n++)
	{
		cgltf_node* Node = Data->nodes + n;

		if (Node->mesh == NULL) continue;

		if (strstr (Node->name, "CS_") == NULL)
		{
			Asset TmpAsset = { 0 };
			strcpy (TmpAsset.Name, Node->name);

			TmpAsset.GraphicsPrimitiveCount = Node->mesh->primitives_count;
			TmpAsset.GraphicsPrimitives = (GraphicsPrimitive*)MyCalloc (Node->mesh->primitives_count, sizeof (GraphicsPrimitive));
			
			for (uint32_t p = 0; p < Node->mesh->primitives_count; p++)
			{
				cgltf_primitive* Primitive = Node->mesh->primitives + p;
				GraphicsPrimitive* CurrentGraphicsPrimitive = TmpAsset.GraphicsPrimitives + p;
				
				PrimitivePtr PrimPtr = { 0 };
				PrimPtr.GraphicsPrimPtr = CurrentGraphicsPrimitive;

				for (uint32_t a = 0; a < Primitive->attributes_count; a++)
				{
					cgltf_attribute* Attribute = Primitive->attributes + a;
					ImportAttribute (&PrimPtr, Attribute, GraphicsPrimitiveType);
				}

				ImportMaterials (FilePath, &PrimPtr, Primitive->material);
				ImportIndices (&PrimPtr, Primitive, GraphicsPrimitiveType);
			}

			memcpy ((Assets + CurrentMeshAssetCount), &TmpAsset, sizeof (Asset));

			++CurrentMeshAssetCount;
		}
	}
}

void ImportPhysicsPrimitives (Asset* Assets, uint32_t AssetCount, cgltf_data* Data)
{
	uint32_t CurrentMeshAssetCount = 0;

	for (uint32_t n = 0; n < Data->nodes_count; n++)
	{
		cgltf_node* Node = Data->nodes + n;

		if (Node->mesh == NULL) continue;

		if (strstr (Node->name, "CS_") != NULL)
		{
			char* Name = Node->name;

			char* First = strtok (Name, "_");
			char* Second = strtok (NULL, "_");
			char* Third = strtok (NULL, "_");

			for (uint32_t a = 0; a < AssetCount; a++)
			{
				Asset* CurrentAsset = Assets + a;

				if (strcmp (CurrentAsset->Name, Second) == 0)
				{
					uint32_t CurrentAssetPhysicsPrimitiveCount = CurrentAsset->PhysicsPrimitiveCount;
					cgltf_mesh* Mesh = Node->mesh;

					uint32_t AdditionalPrimitiveCount = Mesh->primitives_count;

					if (CurrentAsset->PhysicsPrimitiveCount == 0)
					{
						PhysicsPrimitive* Tmp = (PhysicsPrimitive*)realloc (CurrentAsset->PhysicsPrimitives, (CurrentAssetPhysicsPrimitiveCount + AdditionalPrimitiveCount) * sizeof (PhysicsPrimitive));

						if (Tmp != NULL)
						{
							CurrentAsset->PhysicsPrimitives = Tmp;
						}
					}
					else
					{
						CurrentAsset->PhysicsPrimitives = (PhysicsPrimitive*)MyCalloc (CurrentAssetPhysicsPrimitiveCount + AdditionalPrimitiveCount, sizeof (PhysicsPrimitive));
					}

					CurrentAsset->PhysicsPrimitiveCount = CurrentAssetPhysicsPrimitiveCount + AdditionalPrimitiveCount;

					for (uint32_t p = 0; p < Mesh->primitives_count; p++)
					{
						PhysicsPrimitive* CurrentPhysicsPrimitive = (CurrentAsset->PhysicsPrimitives + (CurrentAssetPhysicsPrimitiveCount + p));

						cgltf_primitive* Primitive = Mesh->primitives + p;

						PrimitivePtr PrimPtr = { 0 };
						PrimPtr.PhysicsPrimPtr = CurrentPhysicsPrimitive;

						for (uint32_t a = 0; a < Primitive->attributes_count; a++)
						{
							cgltf_attribute* Attribute = Primitive->attributes + a;
							ImportAttribute (&PrimPtr, Attribute, PhysicsPrimitiveType);
						}

						ImportIndices (&PrimPtr, Primitive, PhysicsPrimitiveType);
					}
				}
			}
		}
	}
}

int ImportAssets (const char* FilePath, Asset** Assets, uint32_t* AssetCount)
{
	cgltf_options Options = { 0 };
	cgltf_data* Data = NULL;

	cgltf_result Result = cgltf_parse_file (&Options, FilePath, &Data);

	if (Result == cgltf_result_success)
	{
		Result = cgltf_load_buffers (&Options, Data, FilePath);

		if (Result == cgltf_result_success)
		{
			Result = cgltf_validate (Data);

			if (Result == cgltf_result_success)
			{
				for (uint32_t n = 0; n < Data->nodes_count; n++)
				{
					cgltf_node* Node = Data->nodes + n;

					if (Node->mesh == NULL) continue;

					if (strstr (Node->name, "CS_") != NULL) continue;

					++(*AssetCount);
				}

				*Assets = (Asset*)MyCalloc ((size_t)(*AssetCount), sizeof (Asset));
				
				ImportGraphicsPrimitives (FilePath, *Assets, Data);
				ImportPhysicsPrimitives (*Assets, *AssetCount, Data);
			}
		}
	}

	cgltf_free (Data);

	return 0;
}
