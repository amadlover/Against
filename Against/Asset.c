#include "Asset.h"
#include "Error.h"

#include "GraphicsUtilities.h"

#include <Windows.h>
#include <Shlwapi.h>
#include <strsafe.h>

#include <string.h>

#include <cgltf.h>
#include <stb_image.h>

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

void ImportAttribute ()
{

}

void ImportGraphicsPrimitives (Asset* Assets, uint32_t AssetCount, cgltf_data* Data)
{
	for (uint32_t n = 0; n < Data->nodes_count; n++)
	{
		uint32_t CurrentMeshAssetCount = 0;

		cgltf_node* Node = Data->nodes + n;

		if (Node->mesh == NULL) continue;

		if (strstr (Node->name, "CS_") == NULL)
		{
			Asset TmpAsset = { 0 };
			strcpy (TmpAsset.Name, Node->name);

			TmpAsset.GraphicsPrimitiveCount = Node->mesh->primitives_count;
			TmpAsset.GraphicsPrimitives = (GraphicsPrimitive*)calloc (Node->mesh->primitives_count, sizeof (GraphicsPrimitive));
			
			for (uint32_t p = 0; p < Node->mesh->primitives_count; p++)
			{
				cgltf_primitive* Primitive = Node->mesh->primitives + p;
				GraphicsPrimitive* CurrentGraphicsPrimitive = TmpAsset.GraphicsPrimitives + p;

				for (uint32_t a = 0; a < Primitive->attributes_count; a++)
				{
					cgltf_attribute* Attribute = Primitive->attributes + a;
					cgltf_accessor* Accessor = Attribute->data;
					cgltf_buffer_view* BufferView = Accessor->buffer_view;

					if (Attribute->type == cgltf_attribute_type_position)
					{
						char* DataStart = (char*)BufferView->buffer->data;
						float* Positions = (float*)(DataStart + Accessor->offset + BufferView->offset);

						CurrentGraphicsPrimitive->PositionSize = BufferView->size;
						CurrentGraphicsPrimitive->Positions = (float*)malloc (BufferView->size);

						memcpy (CurrentGraphicsPrimitive->Positions, Positions, BufferView->size);
					}
					else if (Attribute->type == cgltf_attribute_type_texcoord)
					{
						if (strcmp (Attribute->name, "TEXCOORD_0") == 0)
						{
							char* DataStart = (char*)BufferView->buffer->data;
							float* UVs = (float*)(DataStart + Accessor->offset + BufferView->offset);
							CurrentGraphicsPrimitive->UV0Size = BufferView->size;
							CurrentGraphicsPrimitive->UV0s = (float*)malloc (BufferView->size);

							memcpy (CurrentGraphicsPrimitive->UV0s, UVs, BufferView->size);
						}
					}
					else if (Attribute->type == cgltf_attribute_type_normal)
					{

					}
				}

				cgltf_accessor* Accessor = Primitive->indices;
				CurrentGraphicsPrimitive->IndexCount = Accessor->count;

				cgltf_buffer_view* BufferView = Accessor->buffer_view;

				char* DataStart = (char*)BufferView->buffer->data;

				switch (Accessor->component_type)
				{
				case cgltf_component_type_r_16u:
					CurrentGraphicsPrimitive->IndexSize = BufferView->size * 2;
					CurrentGraphicsPrimitive->Indices = (uint32_t*)malloc (2 * BufferView->size);

					uint16_t* I16 = (uint16_t*)(DataStart + Accessor->offset + BufferView->offset);

					for (uint32_t i = 0; i < Accessor->count; i++)
					{
						CurrentGraphicsPrimitive->Indices[i] = I16[i];
					}

					break;

				case cgltf_component_type_r_32u:
					CurrentGraphicsPrimitive->IndexSize = BufferView->size;
					CurrentGraphicsPrimitive->Indices = (uint32_t*)malloc (BufferView->size);

					uint32_t* I32 = (uint32_t*)(DataStart + Accessor->offset + BufferView->offset);
					memcpy (CurrentGraphicsPrimitive->Indices, I32, BufferView->size);

					break;

				default:
					break;
				}
			}

			memcpy ((Assets + CurrentMeshAssetCount), &TmpAsset, sizeof (Asset));

			++CurrentMeshAssetCount;
		}
	}
}

void ImportPhysicsPrimitives (Asset* Assets, uint32_t AssetCount, cgltf_data* Data)
{
	for (uint32_t n = 0; n < Data->nodes_count; n++)
	{
		uint32_t CurrentMeshAssetCount = 0;
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
						CurrentAsset->PhysicsPrimitives = (PhysicsPrimitive*)calloc (CurrentAssetPhysicsPrimitiveCount + AdditionalPrimitiveCount, sizeof (PhysicsPrimitive));
					}

					CurrentAsset->PhysicsPrimitiveCount = CurrentAssetPhysicsPrimitiveCount + AdditionalPrimitiveCount;

					for (uint32_t p = 0; p < Mesh->primitives_count; p++)
					{
						PhysicsPrimitive* CurrentPhysicsPrimitive = (CurrentAsset->PhysicsPrimitives + (CurrentAssetPhysicsPrimitiveCount + p));

						cgltf_primitive* Primitive = Mesh->primitives + p;

						for (uint32_t a = 0; a < Primitive->attributes_count; a++)
						{
							cgltf_attribute* Attribute = Primitive->attributes + a;
							cgltf_accessor* Accessor = Attribute->data;
							cgltf_buffer_view* BufferView = Accessor->buffer_view;

							if (Attribute->type == cgltf_attribute_type_position)
							{
								char* DataStart = (char*)BufferView->buffer->data;
								float* Positions = (float*)(DataStart + Accessor->offset + BufferView->offset);

								CurrentPhysicsPrimitive->PositionsSize = BufferView->size;
								CurrentPhysicsPrimitive->Positions = (float*)malloc (BufferView->size);

								memcpy (CurrentPhysicsPrimitive->Positions, Positions, BufferView->size);
							}
						}

						cgltf_accessor* Accessor = Primitive->indices;
						CurrentPhysicsPrimitive->IndexCount = Accessor->count;

						cgltf_buffer_view* BufferView = Accessor->buffer_view;

						char* DataStart = (char*)BufferView->buffer->data;

						switch (Accessor->component_type)
						{
						case cgltf_component_type_r_16u:
							CurrentPhysicsPrimitive->IndexSize = BufferView->size * 2;
							CurrentPhysicsPrimitive->Indices = (uint32_t*)malloc (2 * BufferView->size);

							uint16_t* I16 = (uint16_t*)(DataStart + Accessor->offset + BufferView->offset);

							for (uint32_t i = 0; i < Accessor->count; i++)
							{
								CurrentPhysicsPrimitive->Indices[i] = I16[i];
							}

							break;

						case cgltf_component_type_r_32u:
							CurrentPhysicsPrimitive->IndexSize = BufferView->size;
							CurrentPhysicsPrimitive->Indices = (uint32_t*)malloc (BufferView->size);

							uint32_t* I32 = (uint32_t*)(DataStart + Accessor->offset + BufferView->offset);
							memcpy (CurrentPhysicsPrimitive->Indices, I32, BufferView->size);

							break;

						default:
							break;
						}
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

					++(*AssetCount);
				}

				*Assets = (Asset*)calloc ((size_t)(*AssetCount), sizeof (Asset));
				
				ImportGraphicsPrimitives (*Assets, *AssetCount, Data);
				ImportPhysicsPrimitives (*Assets, *AssetCount, Data);
			}
		}
	}

	cgltf_free (Data);

	return 0;
}
