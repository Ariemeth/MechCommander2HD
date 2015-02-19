//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

// Resources.cpp : Defines the entry point for the DLL application.
//
#include <windows.h>

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	(void)lpReserved;
	(void)ul_reason_for_call;
	(void)hModule;
    return TRUE;
}

