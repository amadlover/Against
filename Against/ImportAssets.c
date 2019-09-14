#include "ImportAssets.h"
#include "Error.h"

#include <Windows.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

cgltf_data* ImportGLTF (const char* Filename)
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
		}
	}

	return Data;
}

int ImportMainMenuGLTF (const char* Filename, Mesh* Meshes)
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
				for (cgltf_size m = 0; m < Data->meshes_count; m++)
				{
					uint64_t BufferSize = 0;
					cgltf_mesh* Mesh = Data->meshes + m;

					for (cgltf_size p = 0; p < Mesh->primitives_count; p++)
					{
						cgltf_primitive* Primitive = Mesh->primitives + p;

						for (cgltf_size a = 0; a < Primitive->attributes_count; a++)
						{
							cgltf_attribute* Attribute = Primitive->attributes + a;

							if (strcmp (Attribute->name, "POSITION") == 0)
							{
								cgltf_accessor* Accessor = Attribute->data;
								Meshes[m].Positions = (float*)malloc (Accessor->buffer_view->size);
								memcpy_s (Meshes[m].Positions, Accessor->buffer_view->size, Accessor->buffer_view->buffer->data, Accessor->buffer_view->size);
								Meshes[m].PositionsSize = Accessor->buffer_view->size;
							}
							else if (strcmp (Attribute->name, "TEXCOORD_0") == 0)
							{
								cgltf_accessor* Accessor = Attribute->data;
								Meshes[m].UVs = (float*)malloc (Accessor->buffer_view->size);
								memcpy_s (Meshes[m].Positions, Accessor->buffer_view->size, Accessor->buffer_view->buffer->data, Accessor->buffer_view->size);
								Meshes[m].UVsSize = Accessor->buffer_view->size;
							}
						}

						cgltf_accessor* IndicesAccessor = Primitive->indices;
						Meshes[m].IndexCount = IndicesAccessor->count;
						Meshes[m].Indices = (uint32_t*)malloc (IndicesAccessor->buffer_view->size);
						memcpy_s (Meshes[m].Indices, IndicesAccessor->buffer_view->size, IndicesAccessor->buffer_view->buffer->data, IndicesAccessor->buffer_view->size);
						Meshes[m].IndicesSize = IndicesAccessor->buffer_view->size;
					}

					strncpy_s (Meshes[m].Name, 256, Mesh->name, _TRUNCATE);
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
	}
	else
	{
		return AGAINST_ERROR_GLTF_COULD_NOT_IMPORT;
	}

	cgltf_free (Data);

	return 0;
}