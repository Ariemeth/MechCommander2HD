//******************************************************************************************
//	color.h - This file contains the definitions for the Color Tables
//	MechCommander 2
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef COLOR_H
#define COLOR_H
//----------------------------------------------------------------------------------
// Include Files
#include "dstd.h"

//----------------------------------------------------------------------------------
void initColorTables (void);

void destroyColorTables (void);

DWORD *getColorTable (long tableNum);
//----------------------------------------------------------------------------------
#endif