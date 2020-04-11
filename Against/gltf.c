#include "gltf.h"
#include "utils.h"
#include "error.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

int import_graphics_primitives (cgltf_data* data)
{
    
    for (size_t n = 0; n < data->nodes_count; ++n)
    {
        cgltf_node* node = data->nodes + n;

        if (node->mesh == NULL)
        {
            continue;
        }

        if (strstr (node->name, "CS_") != NULL)
        {
             continue;
        }

        char* name = node->name;
        char* first = strtok (name, "_");
        char* second = strtok (NULL, "_");
        char* third = strtok (NULL, "_");
    }

    return 0;
}

int import_meshes ()
{
    return 0;
}

int import_materials ()
{
    return 0;
}

int import_images ()
{
    return 0;
}

int import_skins ()
{
    return 0;
}

int import_animations ()
{
    return 0;
}

int import_gltf_file (const char* partial_file_path)
{
    cgltf_options options = { 0 };
	cgltf_data* data = NULL;

    char full_file_path[MAX_PATH];
    get_full_file_path (partial_file_path, full_file_path);

    if (cgltf_parse_file (&options, full_file_path, &data) != cgltf_result_success)
    {
        return AGAINST_ERROR_GLTF_IMPORT;
    }

    if (cgltf_load_buffers (&options, data, full_file_path) != cgltf_result_success)
    {
        return AGAINST_ERROR_GLTF_IMPORT;
    }

    if (cgltf_validate (data) != cgltf_result_success)
    {
        return AGAINST_ERROR_GLTF_IMPORT;
    }

    CHECK_AGAINST_RESULT (import_meshes ());
    CHECK_AGAINST_RESULT (import_materials ());
    CHECK_AGAINST_RESULT (import_images ());
    CHECK_AGAINST_RESULT (import_skins ());
    CHECK_AGAINST_RESULT (import_animations ());
    
    cgltf_free (data);

    return 0;
}

void import_gltf_files_from_folder (const char* partial_folder_path)
{
    file_path* file_paths;
    size_t num_files = 0;
    get_files_in_folder (partial_folder_path, &file_paths, &num_files);

    for (size_t f = 0 ; f < num_files; ++f)
    {
        char partial_file_path[MAX_PATH];
        strcpy (partial_file_path, partial_folder_path);
        strcat (partial_file_path, file_paths[f].path);
        import_gltf_file (partial_file_path);
    }

    my_free (file_paths);

    return;
}