//***************************************************************************
//	MCHDUtilities.h -- On-screen debug text and timing functionality
//	MechCommander 2 HD
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef MCHDUTILITIES_H
#define MCHDUTILITIES_H
#ifndef FINAL

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GameOS.hpp>
#include <EString.h>

#define MCHD_DBG_STR_CNT 20

///////////////////////////////////////////////////////////////////////////////////////////////
// Global Instance
///////////////////////////////////////////////////////////////////////////////////////////////

class MCHDUtilities;
extern MCHDUtilities * g_MCHD;

class MCHDUtilities
{
public:

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Global Instance
	///////////////////////////////////////////////////////////////////////////////////////////////

	static void			InitializeInstance(void);

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Debug Text
	///////////////////////////////////////////////////////////////////////////////////////////////

	void				RenderDebugText(void);
	__forceinline void	ToggleDebugText(void)		{ showDebugText = !showDebugText; }
	__forceinline void	ShowDebugText(bool _show)	{ showDebugText = _show; }

	EString				debugStrings[MCHD_DBG_STR_CNT];

	///////////////////////////////////////////////////////////////////////////////////////////////
	// High-Resolution Timing
	///////////////////////////////////////////////////////////////////////////////////////////////

	// List of blocks to profile (a separate set of timing variables for each)
	enum EProfileList
	{
		TIME_WHOLE_FRAME,
		TIME_SECTION,
		TIME_COUNT
	};

	// Per-frame update
	void						UpdateTiming(void);

	// Timer control
	void						Start(unsigned int _index);
	void						Stop(unsigned int _index);
	void						Reset(unsigned int _index);
	void						ResetAll();

	// Get results
	__forceinline float			GetFrameMs(unsigned int _index)			{ return frameTimeDisplay[_index]; }
	__forceinline float			GetAverageMs(unsigned int _index)		{ return averageTimeDisplay[_index]; }
	__forceinline float			GetMaxMs(unsigned int _index)			{ return maxTimeDisplay[_index]; }

	__forceinline unsigned int	GetFrameCallCount(unsigned int _index)	{ return frameCallCountDisplay[_index]; }
	__forceinline float			GetAverageCallCount(unsigned int _index){ return averageCallCountDisplay[_index]; }
	__forceinline unsigned int	GetMaxCallCount(unsigned int _index)	{ return maxCallCount[_index]; }

	__forceinline unsigned int	GetFrameCount(unsigned int _index)		{ return frameCount[_index]; }
	__forceinline float			GetAverageFrameTime(void)				{ return averageFrameTime; }
	__forceinline float			GetAverageFPS(void)						{ return averageFPS; }

private:

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Global Instance
	///////////////////////////////////////////////////////////////////////////////////////////////

	// Private constructors
	MCHDUtilities(const MCHDUtilities &);
	MCHDUtilities(void);

	///////////////////////////////////////////////////////////////////////////////////////////////
	// High-Resolution Timing
	///////////////////////////////////////////////////////////////////////////////////////////////

	void			InitializeTiming(void);

	void			EndFrame(unsigned int _index);
	void			EndFrameAll();

	// Basic timing variables
	LARGE_INTEGER	frequency;					// Performance counter frequency
	LARGE_INTEGER	start[TIME_COUNT];			// Start and stop points
	LARGE_INTEGER	stop[TIME_COUNT];			

	// Tracking variables
	LARGE_INTEGER	callTime[TIME_COUNT];		// Current call time
	LARGE_INTEGER	maxCallTime[TIME_COUNT];	// Largest single call time
	LARGE_INTEGER	frameTime[TIME_COUNT];		// Time profiled in the current frame
	LARGE_INTEGER	maxTime[TIME_COUNT];		// Largest frame time profiled
	LARGE_INTEGER	totalTime[TIME_COUNT];		// The total accumulated call time

	unsigned int	frameCallCount[TIME_COUNT];	// Number of calls during the current frame
	unsigned int	maxCallCount[TIME_COUNT];	// Number of calls during the frame with the highest time
	unsigned int	totalCallCount[TIME_COUNT];	// Number of calls during the entire run

	unsigned int	frameCount[TIME_COUNT];		// Number of times EndFrame() has been called since Reset()

	// Display variables (saves the last full frame of per-frame variables before they get reset)
	float			frameTimeDisplay[TIME_COUNT];
	float			averageTimeDisplay[TIME_COUNT];
	float			maxTimeDisplay[TIME_COUNT];
	unsigned int	frameCallCountDisplay[TIME_COUNT];
	float			averageCallCountDisplay[TIME_COUNT];

	// Average FPS (TIME_WHOLE_FRAME) over the last FRAMES_TO_TRACK frames
#define FRAMES_TO_TRACK 120
	float			frameTimeList[FRAMES_TO_TRACK];
	unsigned int	fpsFrameIndex;
	float			averageFrameTime;					
	float			averageFPS;	

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Debug Text
	///////////////////////////////////////////////////////////////////////////////////////////////

	void			InitializeDebugText(void);

	HGOSFONT3D		font;
	bool			showDebugText;
};

#else

///////////////////////////////////////////////////////////////////////////////////////////////
// Global Instance
///////////////////////////////////////////////////////////////////////////////////////////////

class MCHDUtilities;
extern MCHDUtilities * g_MCHD;

class MCHDUtilities
{
public:

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Global Instance
	///////////////////////////////////////////////////////////////////////////////////////////////

	static void					InitializeInstance(void);

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Debug Text
	///////////////////////////////////////////////////////////////////////////////////////////////

	__forceinline void			RenderDebugText(void) const {}
	__forceinline void			ToggleDebugText(void) const {}
	__forceinline void			ShowDebugText(bool _show) const { (void)_show; }

	///////////////////////////////////////////////////////////////////////////////////////////////
	// High-Resolution Timing
	///////////////////////////////////////////////////////////////////////////////////////////////

	// List of blocks to profile (a separate set of timing variables for each)
	enum EProfileList
	{
		TIME_WHOLE_FRAME,
		TIME_SECTION,
		TIME_COUNT
	};

	// Per-frame update
	__forceinline void			UpdateTiming(void) const {}

	// Timer control
	__forceinline void			Start(unsigned int _index) const { (void)_index; }
	__forceinline void			Stop(unsigned int _index) const { (void)_index; }

	__forceinline void			Reset(unsigned int _index) const { (void)_index; }
	__forceinline void			ResetAll() const {}

	// Get results
	__forceinline float			GetFrameMs(unsigned int _index) const { (void)_index; }
	__forceinline float			GetAverageMs(unsigned int _index) const { (void)_index; }
	__forceinline float			GetMaxMs(unsigned int _index) const { (void)_index; }

	__forceinline unsigned int	GetFrameCallCount(unsigned int _index) const { (void)_index; }
	__forceinline float			GetAverageCallCount(unsigned int _index) const { (void)_index; }
	__forceinline unsigned int	GetMaxCallCount(unsigned int _index) const { (void)_index; }

	__forceinline unsigned int	GetFrameCount(unsigned int _index) const { (void)_index; }
};

#endif
#endif



