#include "Utility.h"
#include <Shlwapi.h>

LinkedList* Ts = NULL;

void GetApplicationFolder (TCHAR* Path)
{
	HMODULE Module = GetModuleHandle (NULL);
	GetModuleFileName (Module, Path, MAX_PATH);
	PathRemoveFileSpec (Path);
}

void SetupLinkedList (int NumElements)
{
	for (int n = 0; n < NumElements; n++)
	{
		
	}
}