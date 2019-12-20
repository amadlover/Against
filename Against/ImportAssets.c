#include "ImportAssets.h"
#include "Error.h"

#include <Windows.h>
#include <strsafe.h>
#include <Shlwapi.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#define STB_IMPLEMENTATION
#include <stb_image.h>

int ImportGLTF (const char* Filename, Node** Nodes, uint32_t* NodeCount, Mesh** Meshes, uint32_t* MeshCount, Material** Materials, uint32_t* MaterialCount, Texture** Textures, uint32_t* TextureCount, Image** Images, uint32_t* ImageCount, Sampler** Samplers, uint32_t* SamplerCount)
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
				*SamplerCount = Data->samplers_count;
				*Samplers = (Sampler*)malloc (sizeof (Sampler) * Data->samplers_count);

				for (uint32_t s = 0; s < Data->samplers_count; s++)
				{
					cgltf_sampler* Sampler = Data->samplers + s;

					(*Samplers + s)->MagFilter = Sampler->mag_filter;
					(*Samplers + s)->MinFilter = Sampler->min_filter;
					(*Samplers + s)->Wrap_S = Sampler->wrap_s;
					(*Samplers + s)->Wrap_T = Sampler->wrap_t;
				}

				*ImageCount = Data->images_count;
				*Images = (Image*)malloc (sizeof (Image) * Data->images_count);
				memset (*Images, 0, sizeof (Image) * Data->images_count);

				for (uint32_t i = 0; i < Data->images_count; i++)
				{
					cgltf_image* Image = Data->images + i;

					TCHAR TextureFile[MAX_PATH];
					mbstowcs (TextureFile, Filename, MAX_PATH);

					PathRemoveFileSpec (TextureFile);
					TCHAR URI[MAX_PATH];
					mbstowcs (URI, Image->uri, MAX_PATH);

					TCHAR URIPath[MAX_PATH];
					StringCchCopy (URIPath, MAX_PATH, L"\\");
					StringCchCat (URIPath, MAX_PATH, URI);

					StringCchCat (TextureFile, MAX_PATH, URIPath);

					char TextureFilename[MAX_PATH];
					wcstombs (TextureFilename, TextureFile, MAX_PATH);
					(*Images + i)->Pixels = stbi_load (TextureFilename, &(*Images + i)->Width, &(*Images + i)->Height, &(*Images + i)->BPP, 4);
					(*Images + i)->Size = (*Images + i)->Width * (*Images + i)->Height * 4 * sizeof (unsigned char);

					if (Image->name)
					{
						strcpy ((*Images + i)->Name, Image->name);
					}
				}

				*TextureCount = Data->textures_count;
				*Textures = (Texture*)malloc (sizeof (Texture) * Data->textures_count);
				memset (*Textures, 0, sizeof (Texture) * Data->textures_count);

				for (uint32_t t = 0; t < Data->textures_count; t++)
				{
					cgltf_texture* Texture = Data->textures + t;

					for (uint32_t i = 0; i < Data->images_count; i++)
					{
						if ((Data->images + i) == (Texture->image))
						{
							(*Textures + t)->Image = (*Images + i);
						}
					}

					for (uint32_t s = 0; s < Data->samplers_count; s++)
					{
						if ((Data->samplers + s) == (Texture->sampler))
						{
							(*Textures + t)->Sampler = (*Samplers + s);
						}
					}

					if (Texture->name)
					{
						strcpy ((*Textures + t)->Name, Texture->name);
					}
				}

				*MaterialCount = Data->materials_count;
				*Materials = (Material*)malloc (sizeof (Material) * Data->materials_count);
				memset (*Materials, 0, sizeof (Material) * Data->materials_count);

				for (uint32_t m = 0; m < Data->materials_count; m++)
				{
					cgltf_material* Material = Data->materials + m;

					for (uint32_t t = 0; t < Data->textures_count; t++)
					{
						if (Material->pbr_metallic_roughness.base_color_texture.texture == (Data->textures + t))
						{
							(*Materials + m)->BaseColorTexture = (*Textures + t);
						}
					}

					if (Material->name)
					{
						strcpy ((*Materials + m)->Name, Material->name);
					}
				}

				*MeshCount = Data->meshes_count;
				*Meshes = (Mesh*)malloc (sizeof (Mesh) * Data->meshes_count);
				memset (*Meshes, 0, sizeof (Mesh) * Data->meshes_count);

				for (uint32_t m = 0; m < Data->meshes_count; m++)
				{
					cgltf_mesh* Mesh = Data->meshes + m;

					(*Meshes + m)->PrimitiveCount = Mesh->primitives_count;
					(*Meshes + m)->Primitives = (Primitive*)malloc (sizeof (Primitive) * Mesh->primitives_count);
					memset ((*Meshes + m)->Primitives, 0, sizeof (Primitive) * Mesh->primitives_count);

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

								(*Meshes + m)->Primitives[p].PositionSize = BufferView->size;
								(*Meshes + m)->Primitives[p].Positions = (float*)malloc (BufferView->size);

								memcpy ((*Meshes + m)->Primitives[p].Positions, Positions, BufferView->size);
							}
							else if (Attribute->type == cgltf_attribute_type_texcoord)
							{
								if (strcmp (Attribute->name, "TEXCOORD_0") == 0)
								{
									char* DataStart = (char*)BufferView->buffer->data;
									float* UVs = (float*)(DataStart + Accessor->offset + BufferView->offset);

									(*Meshes + m)->Primitives[p].UV0Size = BufferView->size;
									(*Meshes + m)->Primitives[p].UV0s = (float*)malloc (BufferView->size);

									memcpy ((*Meshes + m)->Primitives[p].UV0s, UVs, BufferView->size);
								}
							}
						}

						cgltf_accessor* Accessor = Primitive->indices;
						cgltf_buffer_view* BufferView = Accessor->buffer_view;

						(*Meshes + m)->Primitives[p].IndexCount = Accessor->count;

						char* DataStart = (char*)BufferView->buffer->data;

						switch (Accessor->component_type)
						{
						case cgltf_component_type_r_16u:
							(*Meshes + m)->Primitives[p].IndexSize = BufferView->size * 2;
							(*Meshes + m)->Primitives[p].Indices = (uint32_t*)malloc (BufferView->size * 2);

							uint16_t* I16 = (uint16_t*)(DataStart + Accessor->offset + BufferView->offset);

							for (uint32_t i = 0; i < Accessor->count; i++)
							{
								(*Meshes + m)->Primitives[p].Indices[i] = I16[i];
							}

							break;

						case cgltf_component_type_r_32u:
							(*Meshes + m)->Primitives[p].IndexSize = BufferView->size;
							(*Meshes + m)->Primitives[p].Indices = (uint32_t*)malloc (BufferView->size);

							uint32_t* I32 = (uint32_t*)(DataStart + Accessor->offset + BufferView->offset);

							memcpy ((*Meshes + m)->Primitives[p].Indices, I32, BufferView->size);
							break;

						default:
							break;
						}

						for (uint32_t mat = 0; mat < Data->materials_count; mat++)
						{
							if (Primitive->material == (Data->materials + mat))
							{
								(*Meshes + m)->Primitives[p].Material = (*Materials + mat);
							}
						}
					}

					if (Mesh->name)
					{
						strcpy ((*Meshes + m)->Name, Mesh->name);
					}
				}

				*NodeCount = Data->nodes_count;
				*Nodes = (Node*)malloc (sizeof (Node) * Data->nodes_count);
				memset (*Nodes, 0, sizeof (Node) * Data->nodes_count);
				
				for (uint32_t n = 0; n < Data->nodes_count; n++)
				{
					cgltf_node* Node = Data->nodes + n;

					if (Node->has_matrix)
					{
						memcpy ((*Nodes + n)->TransformationMatrix, Node->matrix, sizeof (float) * 16);
					}

					if (Node->has_translation)
					{
						memcpy ((*Nodes + n)->Translation, Node->translation, sizeof (float) * 3);
					}

					if (Node->has_rotation)
					{
						memcpy ((*Nodes + n)->Rotation, Node->rotation, sizeof (float) * 4);
					}

					if (Node->has_scale)
					{
						memcpy ((*Nodes + n)->Scale, Node->scale, sizeof (float) * 3);
					}
					else
					{
						(*Nodes + n)->Scale[0] = 1; (*Nodes + n)->Scale[1] = 1; (*Nodes + n)->Scale[2] = 1;
					}

					if (Node->mesh)
					{
						cgltf_mesh* Mesh = Node->mesh;

						for (uint32_t m = 0; m < Data->meshes_count; m++)
						{
							if (Mesh == (Data->meshes + m))
							{
								(*Nodes + n)->Mesh = (*Meshes + m);
							}
						}
					}

					if (Node->name)
					{
						strcpy ((*Nodes + n)->Name, Node->name);
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