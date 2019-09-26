#include "ImportAssets.h"
#include "Error.h"

#include <Windows.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#define STB_IMPLEMENTATION
#include <stb_image.h>

#include <Shlwapi.h>
#include <strsafe.h>

int ImportMainMenuGLTF (const char* Filename, Mesh** Meshes, uint32_t* MeshCount)
{
	cgltf_options Options = { 0 };
	cgltf_data* Data = NULL;

	cgltf_result Result = cgltf_parse_file (&Options, Filename, &Data);

	if (Result == cgltf_result_success)
	{
		Result = cgltf_load_buffers (&Options, Data, Filename);

		if (Result == cgltf_result_success)
		{
			Result = cgltf_validate (Data);

			if (Result == cgltf_result_success)
			{
				*MeshCount = Data->meshes_count;
				*Meshes = (Mesh*)malloc (sizeof (Mesh) * Data->meshes_count);
				memset (*Meshes, 0, sizeof (Mesh) * Data->meshes_count);

				uint32_t MeshCounter = 0;

				for (uint32_t n = 0; n < Data->nodes_count; n++)
				{
					cgltf_node* Node = Data->nodes + n;

					if (Node->mesh)
					{
						cgltf_mesh* Mesh = Node->mesh;

						if (Node->has_matrix)
						{
							memcpy ((*Meshes + MeshCounter)->TransformationMatrix, Node->matrix, sizeof (float) * 16);
						}

						if (Node->has_translation)
						{
							memcpy ((*Meshes + MeshCounter)->Translation, Node->translation, sizeof (float) * 3);
						}

						if (Node->has_rotation)
						{
							memcpy ((*Meshes + MeshCounter)->Rotation, Node->rotation, sizeof (float) * 4);
						}

						if (Node->has_scale)
						{
							memcpy ((*Meshes + MeshCounter)->Scale, Node->scale, sizeof (float) * 3);
						}

						(*Meshes + MeshCounter)->PrimitiveCount = Mesh->primitives_count;
						(*Meshes + MeshCounter)->Primitives = (Primitive*)malloc (sizeof (Primitive) * Mesh->primitives_count);
						memset ((*Meshes + MeshCounter)->Primitives, 0, sizeof (Primitive) * Mesh->primitives_count);

						for (uint32_t p = 0; p < Mesh->primitives_count; p++)
						{
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

									(*Meshes + MeshCounter)->Primitives[p].PositionsSize = BufferView->size;
									(*Meshes + MeshCounter)->Primitives[p].Positions = (float*)malloc (BufferView->size);

									memcpy ((*Meshes + MeshCounter)->Primitives[p].Positions, Positions, BufferView->size);
								}
								else if (Attribute->type == cgltf_attribute_type_texcoord)
								{
									if (strcmp (Attribute->name, "TEXCOORD_0") == 0)
									{
										char* DataStart = (char*)BufferView->buffer->data;
										float* UVs = (float*)(DataStart + Accessor->offset + BufferView->offset);

										(*Meshes + MeshCounter)->Primitives[p].UVsSize = BufferView->size;
										(*Meshes + MeshCounter)->Primitives[p].UVs = (float*)malloc (BufferView->size);

										memcpy ((*Meshes + MeshCounter)->Primitives[p].UVs, UVs, BufferView->size);
									}
								}
							}

							cgltf_accessor* Accessor = Primitive->indices;
							cgltf_buffer_view* BufferView = Accessor->buffer_view;

							(*Meshes + MeshCounter)->Primitives[p].IndexCount = Accessor->count;


							char* DataStart = (char*)BufferView->buffer->data;

							switch (Accessor->component_type)
							{
							case cgltf_component_type_r_16u:
								(*Meshes + MeshCounter)->Primitives[p].IndicesSize = BufferView->size * 2;
								(*Meshes + MeshCounter)->Primitives[p].Indices = (uint32_t*)malloc (BufferView->size * 2);

								uint16_t* I16 = (uint16_t*)(DataStart + Accessor->offset + BufferView->offset);

								for (uint32_t i = 0; i < Accessor->count; i++)
								{
									(*Meshes + MeshCounter)->Primitives[p].Indices[i] = I16[i];
								}

								break;

							case cgltf_component_type_r_32u:
								(*Meshes + MeshCounter)->Primitives[p].IndicesSize = BufferView->size;
								(*Meshes + MeshCounter)->Primitives[p].Indices = (uint32_t*)malloc (BufferView->size);

								uint32_t* I32 = (uint32_t*)(DataStart + Accessor->offset + BufferView->offset);

								memcpy ((*Meshes + MeshCounter)->Primitives[p].Indices, I32, BufferView->size);
								break;

							default:
								break;
							}
						}

						MeshCounter++;
					}
				}

				cgltf_free (Data);
			}
			else
			{
				return AGAINST_ERROR_GLTF_COULD_NOT_IMPORT;
			}
		}
		else
		{
			return AGAINST_ERROR_GLTF_COULD_NOT_IMPORT;
		}
	}
	else
	{
		return AGAINST_ERROR_GLTF_COULD_NOT_IMPORT;
	}

	return 0;
}