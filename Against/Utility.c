#include "Utility.h"
#include <Shlwapi.h>
#include <strsafe.h>

LinkedList* Ts = NULL;

void GetApplicationFolder (char* OutPath)
{
	TCHAR Path[MAX_PATH];
	HMODULE Module = GetModuleHandle (NULL);
	GetModuleFileName (Module, Path, MAX_PATH);
	PathRemoveFileSpec (Path);

	wcstombs_s (NULL, OutPath, MAX_PATH, Path, MAX_PATH);
}

void GetFullFilePath (char* OutFilePath, char* PartialFilePath)
{
	char Path[MAX_PATH];
	GetApplicationFolder (Path);
	memcpy (OutFilePath, strcat (Path, PartialFilePath), MAX_PATH);
}

void SetupLinkedList (LinkedList SomeList, int NumElements)
{
	for (int n = 0; n < NumElements; n++)
	{
		
	}
}