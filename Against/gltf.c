#include "gltf.h"
#include "utils.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

void import_gltf_file (const char* file_path)
{
    cgltf_options options = { 0 };
	cgltf_data* data = NULL;

	cgltf_result result = cgltf_parse_file (&options, file_path, &data);

    cgltf_free (data);
}

void import_gltf_files_from_folder (const char* partial_folder_path)
{
    file_path* file_paths;
    size_t num_files = 0;
    get_files_in_folder (partial_folder_path, &file_paths, &num_files);

    for (size_t f = 0 ; f < num_files; ++f)
    {
        import_gltf_file (file_paths[f].path);
    }

    my_free (file_paths);

    return;
}