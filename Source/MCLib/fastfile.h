//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//----------------------------------------------------------------------------
// Global Fast File Header. 

#include "dstd.h"
#include "ffile.h"

//-----------------------------------------------------------------------------------
// MechCommander's File classes understand how to deal with the FastFile, once open.
// In other words, the global class needs only to register newly opened fastfiles,
// close them all on exit and provide a find function which returns a pointer to the
// fastfile containing the file we care about.  If no file is found, returns NULL.
extern bool FastFileInit (char *fname);
extern void FastFileFini (void);
extern FastFile *FastFileFind (char *fname, long &fastFileHandle);
extern DWORD elfHash (char *name);
//-----------------------------------------------------------------------------------

