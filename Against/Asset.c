#include "Asset.h"
#include "Error.h"

#include "graphics_utils.h"
#include "utils.h"

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
			PrimPtr->GraphicsPrimPtr->PositionsSize = BufferView->size;
			PrimPtr->GraphicsPrimPtr->Positions = (float*)my_malloc (BufferView->size);
			memcpy (PrimPtr->GraphicsPrimPtr->Positions, Positions, BufferView->size);
		}
		else if (PrimType == PhysicsPrimitiveType)
		{
			PrimPtr->PhysicsPrimPtr->PositionsSize = BufferView->size;
			PrimPtr->PhysicsPrimPtr->Positions = (float*)my_malloc (BufferView->size);
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
				PrimPtr->GraphicsPrimPtr->UV0sSize = BufferView->size;
				PrimPtr->GraphicsPrimPtr->UV0s = (float*)my_malloc (BufferView->size);
				memcpy (PrimPtr->GraphicsPrimPtr->UV0s, UVs, BufferView->size);
			}
		}
	}
}

void ImportMaterials (const char* FilePath, PrimitivePtr* PrimPtr, const cgltf_material* Material, cgltf_data* Data, image* Images)
{
	strcpy (PrimPtr->GraphicsPrimPtr->Material.Name, Material->name);

	if (Material->has_pbr_metallic_roughness)
	{
		if (Material->pbr_metallic_roughness.base_color_texture.texture->name != NULL)
		{
			strcpy (PrimPtr->GraphicsPrimPtr->Material.BaseColorTexture.Name, Material->pbr_metallic_roughness.base_color_texture.texture->name);
		}

		for (uint32_t i = 0; i < Data->images_count; i++)
		{
			cgltf_image* I = Data->images + i;

			if (Material->pbr_metallic_roughness.base_color_texture.texture->image == I)
			{
				PrimPtr->GraphicsPrimPtr->Material.BaseColorTexture.image = Images + i;
			}
		}
	}
}

void ImportIndices (PrimitivePtr* PrimPtr, const cgltf_primitive* Primitive, EPrimitiveType PrimType)
{
	cgltf_accessor* Accessor = Primitive->indices;

	if (PrimType == GraphicsPrimitiveType)
	{
		PrimPtr->GraphicsPrimPtr->IndexCount = Accessor->count;
	}
	else if (PrimType == PhysicsPrimitiveType)
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
			PrimPtr->GraphicsPrimPtr->IndicesSize = (VkDeviceSize)BufferView->size * 2;
			PrimPtr->GraphicsPrimPtr->Indices = (uint32_t*)my_malloc (2 * BufferView->size);
		}
		else if (PrimType == PhysicsPrimitiveType)
		{
			PrimPtr->PhysicsPrimPtr->IndicesSize = (VkDeviceSize)BufferView->size * 2;
			PrimPtr->PhysicsPrimPtr->Indices = (uint32_t*)my_malloc (2 * BufferView->size);
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
			PrimPtr->GraphicsPrimPtr->IndicesSize = BufferView->size;
			PrimPtr->GraphicsPrimPtr->Indices = (uint32_t*)my_malloc (BufferView->size);
		}
		else if (PrimType == PhysicsPrimitiveType)
		{
			PrimPtr->PhysicsPrimPtr->IndicesSize = BufferView->size;
			PrimPtr->PhysicsPrimPtr->Indices = (uint32_t*)my_malloc (BufferView->size);
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

void ImportGraphicsPrimitives (const char* FilePath, Asset* Assets, cgltf_data* Data, image* Images)
{
	uint32_t CurrentMeshAssetCount = 0;

	for (uint32_t n = 0; n < Data->nodes_count; n++)
	{
		cgltf_node* Node_Orig = Data->nodes + n;

		if (Node_Orig->mesh == NULL) continue;

		if (strstr (Node_Orig->name, "CS_") == NULL)
		{
			Asset TmpAsset = { 0 };
			strcpy (TmpAsset.Name, Node_Orig->name);

			TmpAsset.GraphicsPrimitiveCount = Node_Orig->mesh->primitives_count;
			TmpAsset.GraphicsPrimitives = (GraphicsPrimitive*)my_calloc (Node_Orig->mesh->primitives_count, sizeof (GraphicsPrimitive));

			for (uint32_t p = 0; p < Node_Orig->mesh->primitives_count; p++)
			{
				cgltf_primitive* Primitive = Node_Orig->mesh->primitives + p;
				GraphicsPrimitive* CurrentGraphicsPrimitive = TmpAsset.GraphicsPrimitives + p;

				PrimitivePtr PrimPtr = { 0 };
				PrimPtr.GraphicsPrimPtr = CurrentGraphicsPrimitive;

				for (uint32_t a = 0; a < Primitive->attributes_count; a++)
				{
					cgltf_attribute* Attribute = Primitive->attributes + a;
					ImportAttribute (&PrimPtr, Attribute, GraphicsPrimitiveType);
				}

				ImportMaterials (FilePath, &PrimPtr, Primitive->material, Data, Images);
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
		cgltf_node* Node_Orig = Data->nodes + n;

		if (Node_Orig->mesh == NULL) continue;

		if (strstr (Node_Orig->name, "CS_") != NULL)
		{
			char* Name = Node_Orig->name;

			char* First = strtok (Name, "_");
			char* Second = strtok (NULL, "_");
			char* Third = strtok (NULL, "_");

			for (uint32_t a = 0; a < AssetCount; a++)
			{
				Asset* CurrentAsset = Assets + a;

				if (strcmp (CurrentAsset->Name, Second) == 0)
				{
					uint32_t CurrentAssetPhysicsPrimitiveCount = CurrentAsset->PhysicsPrimitiveCount;
					cgltf_mesh* Mesh_Orig = Node_Orig->mesh;

					uint32_t AdditionalPrimitiveCount = Mesh_Orig->primitives_count;

					if (CurrentAsset->PhysicsPrimitiveCount == 0)
					{
						PhysicsPrimitive* Tmp = (PhysicsPrimitive*)my_realloc (CurrentAsset->PhysicsPrimitives, (CurrentAssetPhysicsPrimitiveCount + AdditionalPrimitiveCount) * sizeof (PhysicsPrimitive));

						if (Tmp != NULL)
						{
							CurrentAsset->PhysicsPrimitives = Tmp;
						}
					}
					else
					{
						CurrentAsset->PhysicsPrimitives = (PhysicsPrimitive*)my_calloc (CurrentAssetPhysicsPrimitiveCount + AdditionalPrimitiveCount, sizeof (PhysicsPrimitive));
					}

					CurrentAsset->PhysicsPrimitiveCount = CurrentAssetPhysicsPrimitiveCount + AdditionalPrimitiveCount;

					for (uint32_t p = 0; p < Mesh_Orig->primitives_count; p++)
					{
						PhysicsPrimitive* CurrentPhysicsPrimitive = (CurrentAsset->PhysicsPrimitives + (CurrentAssetPhysicsPrimitiveCount + p));

						cgltf_primitive* Primitive = Mesh_Orig->primitives + p;

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

int ImportAssets (const char* FilePath, Asset** Assets, uint32_t* AssetCount, image* Images)
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
					cgltf_node* Node_Orig = Data->nodes + n;

					if (Node_Orig->mesh == NULL) continue;

					if (strstr (Node_Orig->name, "CS_") != NULL) continue;

					++(*AssetCount);
				}

				*Assets = (Asset*)my_calloc ((size_t)(*AssetCount), sizeof (Asset));

				ImportGraphicsPrimitives (FilePath, *Assets, Data, Images);
				ImportPhysicsPrimitives (*Assets, *AssetCount, Data);
			}
		}
	}

	cgltf_free (Data);

	return 0;
}

void DestroyAssets (Asset* Assets, uint32_t AssetCount)
{
	for (uint32_t a = 0; a < AssetCount; a++)
	{
		for (uint32_t a = 0; a < AssetCount; a++)
		{
			Asset* CurrentAsset = Assets + a;

			if (CurrentAsset->GraphicsPrimitives)
			{
				for (uint32_t i = 0; i < CurrentAsset->GraphicsPrimitiveCount; i++)
				{
					GraphicsPrimitive* CurrentGP = CurrentAsset->GraphicsPrimitives + i;

					if (CurrentGP->Indices)
					{
						my_free (CurrentGP->Indices);
					}

					if (CurrentGP->Positions)
					{
						my_free (CurrentGP->Positions);
					}

					if (CurrentGP->UV0s)
					{
						my_free (CurrentGP->UV0s);
					}

					if (CurrentGP->Normals)
					{
						my_free (CurrentGP->Normals);
					}

					my_free (CurrentGP);
				}
			}

			if (CurrentAsset->PhysicsPrimitives)
			{
				for (uint32_t i = 0; i < CurrentAsset->PhysicsPrimitiveCount; i++)
				{
					PhysicsPrimitive* CurrentPP = CurrentAsset->PhysicsPrimitives + i;

					if (CurrentPP->Indices)
					{
						my_free (CurrentPP->Indices);
					}

					if (CurrentPP->Positions)
					{
						my_free (CurrentPP->Positions);
					}

					my_free (CurrentPP);
				}
			}
		}
	}

	my_free (Assets);
}