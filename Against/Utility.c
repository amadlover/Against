#include "Utility.h"
#include <Shlwapi.h>
#include <strsafe.h>

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

void MyFree (void* Ptr)
{
	free (Ptr);
}