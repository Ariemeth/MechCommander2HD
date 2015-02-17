//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include "MLRHeaders.hpp"

#if !defined(MLR_MLRCLIPTRICK_HPP)
	#include <MLR\MLRClipTrick.hpp>
#endif

//#############################################################################
//#########################    MLRLineCloud    ###############################
//#############################################################################

MLRLineCloud::ClassData*
	MLRLineCloud::DefaultData = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLineCloud::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == StaticHeap);
	DefaultData =
		new ClassData(
			MLRLineCloudClassID,
			"MidLevelRenderer::MLRLineCloud",
			MLREffect::DefaultData
		);
	Register_Object(DefaultData);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLineCloud::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRLineCloud::MLRLineCloud(int nr, int _type) :
	MLREffect(2*nr, DefaultData), type(_type)
{
	Verify(gos_GetCurrentHeap() == Heap);
	usedNrOfVertices = 0;

	Check_Pointer(this);
	
	drawMode = SortData::LineCloud;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRLineCloud::~MLRLineCloud()
{
	Check_Object(this);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void 
	MLRLineCloud::SetData(
		const int *count,
		const Stuff::Point3D *point_data,
		const Stuff::RGBAColor *color_data
	)
{
	Check_Pointer(this);

	usedNrOfVertices = count;
	Verify(*usedNrOfVertices <= maxNrOf);
	points = point_data;
	colors = color_data;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void 
	MLRLineCloud::Draw (DrawEffectInformation *dInfo, GOSVertexPool *allVerticesToDraw, MLRSorter *sorter)
{
	Check_Object(this);

	worldToEffect.Invert(*dInfo->effectToWorld);

	Transform(*usedNrOfVertices, 1);

	if( Clip(dInfo->clippingFlags, allVerticesToDraw) )
	{
		sorter->AddEffect(this, dInfo->state);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void 
	MLRLineCloud::Transform(int, int)
{
	Check_Object(this);

	Start_Timer(Transform_Time);
	int i;

	for(i=0;i<*usedNrOfVertices;i++)
	{
		if(IsOn(i) == false)
		{
			continue;
		}

		(*transformedCoords)[i].Multiply(points[i], effectToClipMatrix);
	}
	Stop_Timer(Transform_Time);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int 
	MLRLineCloud::Clip(MLRClippingState clippingFlags, GOSVertexPool *vt)
{
	//--------------------------------------
	// See if we don't have to draw anything
	//--------------------------------------
	//
	int i;
	numGOSVertices = 0;

	if(clippingFlags.GetClippingState() == 0 || usedNrOfVertices <= 0)
	{
		if(usedNrOfVertices <= 0)
		{
			visible = 0;
		}
		else
		{
			Check_Object(vt);
			gos_vertices = vt->GetActualVertexPool();

			for(i=0;i<*usedNrOfVertices;i++)
			{
				if(IsOn(i) == false)
				{
					continue;
				}

				GOSCopyData(
					&gos_vertices[numGOSVertices],
					transformedCoords->GetData(),
					colors,
 					i
				);

				numGOSVertices++;
			}

			Check_Object(vt);
			vt->Increase(numGOSVertices);
			
			visible = numGOSVertices ? 1 : 0;
		}
		return visible;
	}

	Check_Object(vt);
	gos_vertices = vt->GetActualVertexPool();

	Stuff::Vector4D *v4d = transformedCoords->GetData();

	MLRClippingState theAnd, theOr, startClip, endClip;

	for(i=0;i<*usedNrOfVertices;i+=2)
	{
		startClip.Clip4dVertex(v4d+i);
		endClip.Clip4dVertex(v4d+i+1);

		GOSCopyData(&gos_vertices[numGOSVertices], v4d, colors, i);
		numGOSVertices++;

		GOSCopyData(&gos_vertices[numGOSVertices], v4d, colors, i + 1);
		numGOSVertices++;
	}
	vt->Increase(numGOSVertices);

	visible = numGOSVertices ? 1 : 0;

	return visible;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void 
	MLRLineCloud::TestInstance() const
{
	if (usedNrOfVertices)
	{
		Verify(*usedNrOfVertices >= 0);
		Verify(*usedNrOfVertices <= maxNrOf);
	}
}

