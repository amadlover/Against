#include "utils.h"
#include <Shlwapi.h>
#include <strsafe.h>

void GetFullTexturePathFromURI (const char* FilePath, const char* URI, char* OutFullTexturePath)
{
	TCHAR TextureFile[MAX_PATH];
	mbstowcs (TextureFile, FilePath, MAX_PATH);

	PathRemoveFileSpec (TextureFile);
	TCHAR Uri[MAX_PATH];
	mbstowcs (Uri, URI, MAX_PATH);

	TCHAR URIPath[MAX_PATH];
	StringCchCopy (URIPath, MAX_PATH, L"\\");
	StringCchCat (URIPath, MAX_PATH, Uri);

	StringCchCat (TextureFile, MAX_PATH, URIPath);
	wcstombs (OutFullTexturePath, TextureFile, MAX_PATH);
}

void GetFullFilePath (char* OutFilePath, char* PartialFilePath)
{
	char Path[MAX_PATH];

	TCHAR TPath[MAX_PATH];
	HMODULE Module = GetModuleHandle (NULL);
	GetModuleFileName (Module, TPath, MAX_PATH);
	PathRemoveFileSpec (TPath);

	wcstombs_s (NULL, Path, MAX_PATH, TPath, MAX_PATH);
	memcpy (OutFilePath, strcat (Path, PartialFilePath), MAX_PATH);
}

void* MyMalloc (size_t Size)
{
	return malloc (Size);
}

void* MyCalloc (size_t Count, size_t Size)
{
	return calloc (Count, Size);
}

void* MyRealloc (void* Ptr, size_t Size)
{
	return realloc (Ptr, Size);
}

void MyFree (void* Ptr)
{
	free (Ptr);
}