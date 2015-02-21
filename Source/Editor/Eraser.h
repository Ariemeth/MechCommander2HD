#ifndef ERASER_H
#define ERASER_H
/*************************************************************************************************\
Eraser.h			: Interface for the Eraser component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include "Brush.h"

//*************************************************************************************************

/**************************************************************************************************
CLASS DESCRIPTION
Eraser:
**************************************************************************************************/
#include "EList.h"
#include "Action.h"
#include "BuildingBrush.h"
#include "LinkBrush.h"
#include "EditorInterface.h" /*only for the definition of TeamsAction*/

class Eraser: public Brush
{
	public:

		Eraser(){ pCurAction = NULL; }

		virtual bool beginPaint();
		virtual Action* endPaint();
		virtual bool paint( Stuff::Vector3D& worldPos, int screenX, int screenY );
		virtual bool canPaint( Stuff::Vector3D& worldPos, int screenX, int screenY, int flags );
		virtual Action* applyToSelection();

	private:

		class EraserAction : public ActionPaintTile
		{
		public:
			EraserAction() { teamsActionIsSet = false; }

			virtual bool undo();
			virtual bool redo();

			BuildingBrush::BuildingAction bldgAction;
			LinkBrush::LinkAction linkAction;
			/*deleting a building that is referred to by an objective condition requires a TeamAction
			to record the objective change*/
			bool teamsActionIsSet;
			TeamsAction teamsAction;
		};

		EraserAction* pCurAction;
};

//*************************************************************************************************
#endif  // end of file ( Eraser.h )
