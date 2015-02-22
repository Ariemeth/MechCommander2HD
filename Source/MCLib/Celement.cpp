//---------------------------------------------------------------------------
// celement.cpp - This file contains the class declarations for the element pool
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//---------------------------------------------------------------------------
// Include files

#include "celement.h"
#include "heap.h"
#include <gameos.hpp>

//---------------------------------------------------------------------------
// Static Data Members
//---------------------------------------------------------------------------
// Class Element
Element::Element (long _depth)
{
	depth = _depth;
	drawNormal = TRUE;
}
//---------------------------------------------------------------------------
// Class Element
Element::Element (float _depth)
{
	depth = _depth;
	drawNormal = TRUE;
}
	
//---------------------------------------------------------------------------
