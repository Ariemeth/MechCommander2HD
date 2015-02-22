//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//----------------------------------------------------------------------------
// Global Fast File Code.

#include "heap.h"
#include "fastfile.h"
#include <ctype.h>

long ffLastError = 0;

#define NO_ERR		0
//-----------------------------------------------------------------------------------
bool FastFileInit (char *fname)
{
	if (g_numFastFiles == g_maxFastFiles)
	{
		ffLastError = -1;
		return FALSE;
	}

	//-----------------------------------------------------------------------------
	//-- Open this fast file, add it to the list O pointers and return TRUE if OK!
	g_fastFiles[g_numFastFiles] = new FastFile;
	long result = g_fastFiles[g_numFastFiles]->open(fname);
	if (result == FASTFILE_VERSION)
	{
		ffLastError = result;
		return FALSE;
	}

	g_numFastFiles++;

	return TRUE;
}

//-----------------------------------------------------------------------------------
void FastFileFini (void)
{
	if (g_fastFiles)
	{
		long currentFastFile = 0;
		while (currentFastFile < g_maxFastFiles)
		{
			if (g_fastFiles[currentFastFile])
			{
				g_fastFiles[currentFastFile]->close();
	
				delete g_fastFiles[currentFastFile];
				g_fastFiles[currentFastFile] = NULL;
			}
	
			currentFastFile++;
		}
	}

	free(g_fastFiles);
	g_fastFiles = NULL;
	g_numFastFiles= 0;
}

//-----------------------------------------------------------------------------------
FastFile *FastFileFind (char *fname, long &fastFileHandle)
{
	if (g_fastFiles)
	{
		DWORD thisHash = elfHash(fname);
		long currentFastFile = 0;
		long tempHandle = -1;
		while (currentFastFile < g_numFastFiles)
		{
			tempHandle = g_fastFiles[currentFastFile]->openFast(thisHash,fname);
			if (tempHandle != -1)
			{
				fastFileHandle = tempHandle;
				return g_fastFiles[currentFastFile];
			}

			currentFastFile++;
		}
	}

	return NULL;
}

//------------------------------------------------------------------
DWORD elfHash (char *name)
{
    unsigned long   h = 0, g;
    while ( *name )
    {
        h = ( h << 4 ) + *name++;
		g = h & 0xF0000000;
		if (g)
            h ^= g >> 24;
        h &= ~g;
    }
    return h;
}

//-----------------------------------------------------------------------------------

