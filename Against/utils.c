#include "utils.h"
#include <Shlwapi.h>
#include <strsafe.h>

#pragma comment (lib, "Shlwapi.lib")

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

void get_full_file_path (char* out_file_path, char* partial_file_path)
{
	char path[MAX_PATH];

	wchar_t t_path[MAX_PATH];
	HMODULE module = GetModuleHandle (NULL);
	GetModuleFileName (module, t_path, MAX_PATH);
	PathRemoveFileSpec (t_path);

	wcstombs_s (NULL, path, MAX_PATH, t_path, MAX_PATH);
	strcpy (out_file_path, path);
	strcat (out_file_path, partial_file_path);
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
	free (ptr);
}