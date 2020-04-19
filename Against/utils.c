#include "utils.h"
#include <Shlwapi.h>
#include <strsafe.h>

#pragma comment (lib, "Shlwapi.lib")

#define STB_IMAGE_IMPLEMENTATION
#define STB_ONLY_PNG
#define STB_ONLY_TGA

#include <stb_image.h>

void get_full_texture_path_from_uri (const char* file_path, const char* uri, char* out_full_texture_path)
{
	wchar_t texture_file[MAX_PATH];
	mbstowcs (texture_file, file_path, MAX_PATH);

	PathRemoveFileSpec (texture_file);
	wchar_t t_uri[MAX_PATH];
	mbstowcs (t_uri, uri, MAX_PATH);

	wchar_t uri_path[MAX_PATH];
	StringCchCopy (uri_path, MAX_PATH, L"\\");
	StringCchCat (uri_path, MAX_PATH, t_uri);

	StringCchCat (texture_file, MAX_PATH, uri_path);
	wcstombs (out_full_texture_path, texture_file, MAX_PATH);
}

void get_full_file_path (const char* partial_file_path, char* out_file_path)
{
	char path[MAX_PATH];

	wchar_t t_path[MAX_PATH];
	HMODULE module = GetModuleHandle (NULL);
	GetModuleFileName (module, t_path, MAX_PATH);
	PathRemoveFileSpec (t_path);

	wcstombs_s (NULL, path, MAX_PATH, t_path, MAX_PATH);
	strcpy (out_file_path, path);
	strcat (out_file_path, "\\");
	strcat (out_file_path, partial_file_path);
}

void get_files_in_folder (const char* partial_folder_path, file_path** out_file_paths, size_t* out_file_count)
{
	char full_folder_path[MAX_PATH];
	get_full_file_path (partial_folder_path, full_folder_path);
	strcat (full_folder_path, "*");

	wchar_t folder_path[MAX_PATH];
	mbstowcs (folder_path, full_folder_path, MAX_PATH);

	WIN32_FIND_DATA ffd;
	HANDLE find_handle = INVALID_HANDLE_VALUE;

	find_handle = FindFirstFile (folder_path, &ffd);
	size_t file_count = 0;
	do
	{
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			char file_name[MAX_PATH];
			wcstombs (file_name, ffd.cFileName, MAX_PATH);
			char* base_name = strtok (file_name, ".");
			char* ext = strtok (NULL, ".");

			if (strcmp (ext, "glb") == 0 || strcmp (ext, "gltf") == 0)
			{
 				++file_count;
			}
		}
	} while (FindNextFile (find_handle, &ffd) != 0);

	*out_file_count = file_count;
	*out_file_paths = (file_path*) my_calloc (file_count, sizeof (file_path));

	find_handle = FindFirstFile (folder_path, &ffd);
	size_t current_file_index = 0;
	do
	{
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			char file_name[MAX_PATH];
			wcstombs (file_name, ffd.cFileName, MAX_PATH);
			char* base_name = strtok (file_name, ".");
			char* ext = strtok (NULL, ".");

			if (strcmp (ext, "glb") == 0 || strcmp (ext, "gltf") == 0)
			{
				file_path* current_file_path = *out_file_paths + current_file_index;
				strcpy (current_file_path->path, base_name);
				strcat (current_file_path->path, ".");
				strcat (current_file_path->path, ext);
				++current_file_index;
			}
		}
	} while (FindNextFile (find_handle, &ffd) != 0);
}

void read_image_from_uri (const char* file_path, const char* uri, int* width, int* height, int* bpp, uint8_t* pixels)
{
	char full_path[MAX_PATH];
	get_full_texture_path_from_uri (file_path, uri, full_path);
	pixels = stbi_load (full_path, width, height, bpp, 4);
}

void* my_malloc (size_t size)
{
	return malloc (size);
}

void* my_calloc (size_t count, size_t size)
{
	return calloc (count, size);
}

void* my_realloc (void* ptr, size_t size)
{
	return realloc (ptr, size);
}

void my_free (void* ptr)
{
	if (ptr != NULL)
	{
		free (ptr);
		ptr = NULL;
	}
}

void free_image_data (uint8_t* pixels)
{
	stbi_image_free (pixels);
}