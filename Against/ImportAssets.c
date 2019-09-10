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