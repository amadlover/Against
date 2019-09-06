#include "MainMenu.h"
#include <Windows.h>

#include "Error.h"

int ImportMainMenuAssets ()
{
	OutputDebugString (L"ImportMainMenuAssets\n");

	return 0;
}

int SetupMainMenu ()
{
	OutputDebugString (L"SetupMainMenu\n");

	int Result = ImportMainMenuAssets ();
	
	if (Result != 0)
	{
		return Result;
	}

	return 0;
}

int DrawMainMenu ()
{
	return 0;
}

void DestroyMainMenu ()
{
	OutputDebugString (L"DestroyMainMenu\n");

}