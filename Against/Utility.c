#include "Utility.h"
#include <Shlwapi.h>

void GetApplicationFolder (TCHAR* Path)
{
	HMODULE Module = GetModuleHandle (NULL);
	GetModuleFileName (Module, Path, MAX_PATH);
	PathRemoveFileSpec (Path);
}