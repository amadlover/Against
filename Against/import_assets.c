#include "import_assets.h"
#include "error.h"
#include "utils.h"

#include <Windows.h>
#include <strsafe.h>
#include <Shlwapi.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#define STB_IMPLEMENTATION
#include <stb_image.h>

int ImportGLTF (const char* FilePath,
	Node_Orig** Nodes, uint32_t* NodeCount,
	Mesh_Orig** Meshes, uint32_t* MeshCount,
	Material_Orig** Materials, uint32_t* MaterialCount,
	Texture_Orig** Textures, uint32_t* TextureCount,
	Image_Orig** Images, uint32_t* ImageCount,
	Sampler_Orig** Samplers, uint32_t* SamplerCount)
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
				*SamplerCount = Data->samplers_count;
				*Samplers = (Sampler_Orig*)my_malloc (Data->samplers_count * sizeof (Sampler_Orig));

				for (uint32_t s = 0; s < Data->samplers_count; s++)
				{
					cgltf_sampler* Sampler_Orig = Data->samplers + s;

					(*Samplers + s)->MagFilter = Sampler_Orig->mag_filter;
					(*Samplers + s)->MinFilter = Sampler_Orig->min_filter;
					(*Samplers + s)->Wrap_S = Sampler_Orig->wrap_s;
					(*Samplers + s)->Wrap_T = Sampler_Orig->wrap_t;
				}

				*ImageCount = Data->images_count;
				*Images = (Image_Orig*)my_malloc (Data->images_count * sizeof (Image_Orig));

				for (uint32_t i = 0; i < Data->images_count; i++)
				{
					cgltf_image* image = Data->images + i;

					TCHAR TextureFile[MAX_PATH];
					mbstowcs (TextureFile, FilePath, MAX_PATH);

					PathRemoveFileSpec (TextureFile);
					TCHAR URI[MAX_PATH];
					mbstowcs (URI, image->uri, MAX_PATH);

					TCHAR URIPath[MAX_PATH];
					StringCchCopy (URIPath, MAX_PATH, L"\\");
					StringCchCat (URIPath, MAX_PATH, URI);

					StringCchCat (TextureFile, MAX_PATH, URIPath);

					char TextureFilename[MAX_PATH];
					wcstombs (TextureFilename, TextureFile, MAX_PATH);
					(*Images + i)->Pixels = stbi_load (TextureFilename, &(*Images + i)->Width, &(*Images + i)->Height, &(*Images + i)->BPP, 4);
					(*Images + i)->Size = (*Images + i)->Width * (*Images + i)->Height * 4;

					if (image->name)
					{
						strcpy ((*Images + i)->Name, image->name);
					}
				}

				*TextureCount = Data->textures_count;
				*Textures = (Texture_Orig*)my_malloc (Data->textures_count * sizeof (Texture_Orig));

				for (uint32_t t = 0; t < Data->textures_count; t++)
				{
					cgltf_texture* Texture = Data->textures + t;

					for (uint32_t i = 0; i < Data->images_count; i++)
					{
						if ((Data->images + i) == (Texture->image))
						{
							(*Textures + t)->image = (*Images + i);
						}
					}

					for (uint32_t s = 0; s < Data->samplers_count; s++)
					{
						if ((Data->samplers + s) == (Texture->sampler))
						{
							(*Textures + t)->Sampler_Orig = (*Samplers + s);
						}
					}

					if (Texture->name)
					{
						strcpy ((*Textures + t)->Name, Texture->name);
					}
				}

				*MaterialCount = Data->materials_count;
				*Materials = (Material_Orig*)my_malloc (Data->materials_count * sizeof (Material_Orig));

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
				*Meshes = (Mesh_Orig*)my_malloc (Data->meshes_count * sizeof (Mesh_Orig));

				for (uint32_t m = 0; m < Data->meshes_count; m++)
				{
					cgltf_mesh* Mesh_Orig = Data->meshes + m;

					(*Meshes + m)->PrimitiveCount = Mesh_Orig->primitives_count;
					(*Meshes + m)->Primitives = (Primitive_Orig*)my_malloc (Mesh_Orig->primitives_count * sizeof (Primitive_Orig));

					for (uint32_t p = 0; p < Mesh_Orig->primitives_count; p++)
					{
						cgltf_primitive* Primitive = Mesh_Orig->primitives + p;

						for (uint32_t a = 0; a < Primitive->attributes_count; a++)
						{
							cgltf_attribute* Attribute = Primitive->attributes + a;
							cgltf_accessor* Accessor = Attribute->data;
							cgltf_buffer_view* BufferView = Accessor->buffer_view;

							if (Attribute->type == cgltf_attribute_type_position)
							{
								char* DataStart = (char*)BufferView->buffer->data;
								float* Positions = (float*)(DataStart + Accessor->offset + BufferView->offset);

								(*Meshes + m)->Primitives[p].PositionsSize = BufferView->size;
								(*Meshes + m)->Primitives[p].Positions = (float*)my_malloc (BufferView->size);

								memcpy ((*Meshes + m)->Primitives[p].Positions, Positions, BufferView->size);
							}
							else if (Attribute->type == cgltf_attribute_type_texcoord)
							{
								if (strcmp (Attribute->name, "TEXCOORD_0") == 0)
								{
									char* DataStart = (char*)BufferView->buffer->data;
									float* UVs = (float*)(DataStart + Accessor->offset + BufferView->offset);

									(*Meshes + m)->Primitives[p].UV0Size = BufferView->size;
									(*Meshes + m)->Primitives[p].UV0s = (float*)my_malloc (BufferView->size);

									memcpy ((*Meshes + m)->Primitives[p].UV0s, UVs, BufferView->size);
								}
								else if (strcmp (Attribute->name, "TEXCOORD_1") == 0)
								{

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
							(*Meshes + m)->Primitives[p].IndicesSize = BufferView->size * 2;
							(*Meshes + m)->Primitives[p].Indices = (uint32_t*)my_malloc (BufferView->size * 2);

							uint16_t* I16 = (uint16_t*)(DataStart + Accessor->offset + BufferView->offset);

							for (uint32_t i = 0; i < Accessor->count; i++)
							{
								(*Meshes + m)->Primitives[p].Indices[i] = I16[i];
							}

							break;

						case cgltf_component_type_r_32u:
							(*Meshes + m)->Primitives[p].IndicesSize = BufferView->size;
							(*Meshes + m)->Primitives[p].Indices = (uint32_t*)my_malloc (BufferView->size);

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

					if (Mesh_Orig->name)
					{
						strcpy ((*Meshes + m)->Name, Mesh_Orig->name);
					}
				}

				*NodeCount = Data->nodes_count;
				*Nodes = (Node_Orig*)my_malloc (Data->nodes_count * sizeof (Node_Orig));

				for (uint32_t n = 0; n < Data->nodes_count; n++)
				{
					cgltf_node* Node_Orig = Data->nodes + n;

					if (Node_Orig->has_matrix)
					{
						memcpy ((*Nodes + n)->TransformationMatrix, Node_Orig->matrix, sizeof (float) * 16);
					}

					if (Node_Orig->has_translation)
					{
						memcpy ((*Nodes + n)->Translation, Node_Orig->translation, sizeof (float) * 3);
					}

					if (Node_Orig->has_rotation)
					{
						memcpy ((*Nodes + n)->Rotation, Node_Orig->rotation, sizeof (float) * 4);
					}

					if (Node_Orig->has_scale)
					{
						memcpy ((*Nodes + n)->Scale, Node_Orig->scale, sizeof (float) * 3);
					}
					else
					{
						(*Nodes + n)->Scale[0] = 1; (*Nodes + n)->Scale[1] = 1; (*Nodes + n)->Scale[2] = 1;
					}

					if (Node_Orig->mesh)
					{
						cgltf_mesh* Mesh_Orig = Node_Orig->mesh;

						for (uint32_t m = 0; m < Data->meshes_count; m++)
						{
							if (Mesh_Orig == (Data->meshes + m))
							{
								(*Nodes + n)->Mesh_Orig = (*Meshes + m);
							}
						}
					}

					if (Node_Orig->name)
					{
						strcpy ((*Nodes + n)->Name, Node_Orig->name);
					}
				}

				cgltf_free (Data);
			}
			else
			{
				return AGAINST_ERROR_GLTF_IMPORT;
			}
		}
		else
		{
			return AGAINST_ERROR_GLTF_IMPORT;
		}
	}
	else
	{
		return AGAINST_ERROR_GLTF_IMPORT;
	}

	return 0;
}