#include <MCHDUtilities.h>
#ifndef FINAL

///////////////////////////////////////////////////////////////////////////////////////////////////
// Global Instance
///////////////////////////////////////////////////////////////////////////////////////////////////

MCHDUtilities * g_MCHD = nullptr;

void MCHDUtilities::InitializeInstance(void)
{
	g_MCHD = new MCHDUtilities();
	g_MCHD->InitializeTiming();
	g_MCHD->ResetAll();
	g_MCHD->InitializeDebugText();
}

MCHDUtilities::MCHDUtilities()
	: showDebugText(false)
	, font(nullptr)
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// On-Screen Debug Text
///////////////////////////////////////////////////////////////////////////////////////////////////

void MCHDUtilities::InitializeDebugText(void)
{
	font = gos_LoadFont("assets\\graphics\\arial8.tga");
}

void MCHDUtilities::RenderDebugText()
{
	if (showDebugText == false)
		return;

	float scale = 1.5f;
	gos_TextSetAttributes(font, 0xffffffff, scale, true, true, true, false);
	gos_TextSetRegion(0, 0, Environment.screenWidth, Environment.screenHeight);
	for (int i = 0; i < MCHD_DBG_STR_CNT; ++i)
	{
		if (debugStrings[i] != nullptr)
		{
			gos_TextSetPosition(40, (int)(40 + i * scale * 15));
			gos_TextDraw(debugStrings[i]);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Timing Stuff
///////////////////////////////////////////////////////////////////////////////////////////////////

void MCHDUtilities::UpdateTiming()
{
	EndFrameAll();

	frameTimeList[fpsFrameIndex] = frameTimeDisplay[TIME_WHOLE_FRAME];
	if (++fpsFrameIndex >= FRAMES_TO_TRACK)
		fpsFrameIndex = 0;

	float totalTime = 0.0f;
	for (int i = 0; i < FRAMES_TO_TRACK; ++i)
		totalTime += frameTimeList[i];

	averageFrameTime = totalTime / FRAMES_TO_TRACK;
	averageFPS = 1000.0f / averageFrameTime;
}

void MCHDUtilities::InitializeTiming()
{
	QueryPerformanceFrequency(&frequency);
}

void MCHDUtilities::Start(unsigned int _index)
{
	++frameCallCount[_index];
	QueryPerformanceCounter(&start[_index]);
}

void MCHDUtilities::Stop(unsigned int _index)
{
	QueryPerformanceCounter(&stop[_index]);

	// Update call time and max call time
	callTime[_index].QuadPart = stop[_index].QuadPart - start[_index].QuadPart;
	if (callTime[_index].QuadPart > maxCallTime[_index].QuadPart)
		maxCallTime[_index].QuadPart = callTime[_index].QuadPart;

	// Increment the total frame time
	frameTime[_index].QuadPart += callTime[_index].QuadPart;
}

void MCHDUtilities::EndFrame(unsigned int _index)
{
	// If this was the longest frame recorded
	if (frameTime[_index].QuadPart > maxTime[_index].QuadPart)
	{
		// Update the max frame time and associated count
		maxTime[_index].QuadPart = frameTime[_index].QuadPart;
		maxCallCount[_index] = frameCallCount[_index];
		maxTimeDisplay[_index] = (1000000 * maxTime[_index].QuadPart / frequency.QuadPart) / 1000.0f;
	}

	// Increment totals counters
	++frameCount[_index];
	totalCallCount[_index] += frameCallCount[_index];
	totalTime[_index].QuadPart += frameTime[_index].QuadPart;

	// Update the display variable
	frameTimeDisplay[_index] = (1000000 * frameTime[_index].QuadPart / frequency.QuadPart) / 1000.0f;
	averageTimeDisplay[_index] = (1000000 * totalTime[_index].QuadPart / frequency.QuadPart) / (frameCount[_index] * 1000.0f);
	frameCallCountDisplay[_index] = frameCallCount[_index];
	averageCallCountDisplay[_index] = totalCallCount[_index] / float(frameCount[_index]);

	// Reset per-frame variables
	frameTime[_index].QuadPart = 0;
	frameCallCount[_index] = 0;
}

void MCHDUtilities::EndFrameAll()
{
	for (unsigned int iProfile = 0; iProfile < TIME_COUNT; ++iProfile)
		EndFrame(iProfile);
}

void MCHDUtilities::Reset(unsigned int _index)
{
	// Reset all variables
	callTime[_index].QuadPart = 0;
	maxCallTime[_index].QuadPart = 0;

	frameTime[_index].QuadPart = 0;
	totalTime[_index].QuadPart = 0;
	maxTime[_index].QuadPart = 0;

	frameCallCount[_index] = 0;
	totalCallCount[_index] = 0;
	maxCallCount[_index] = 0;

	frameCount[_index] = 0;
}

void MCHDUtilities::ResetAll()
{
	for (unsigned int iProfile = 0; iProfile < TIME_COUNT; ++iProfile)
		Reset(iProfile);
}

#else

///////////////////////////////////////////////////////////////////////////////////////////////////
// Global Instance
///////////////////////////////////////////////////////////////////////////////////////////////////

MCHDUtilities * g_MCHD = nullptr;

void MCHDUtilities::InitializeInstance(void)
{
	g_MCHD = new MCHDUtilities();
}

#endif
