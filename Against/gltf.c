#include "gltf.h"
#include "utils.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

void import_gltf_file (const char* partial_file_path)
{

}

void import_gltf_files_from_folder (const char* partial_folder_path)
{
    get_files_in_folder (partial_folder_path, NULL);

    return;
}