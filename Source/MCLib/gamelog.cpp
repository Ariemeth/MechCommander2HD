//---------------------------------------------------------------------------
//
// gamelog.cpp - This file contains the class functions for GameLog
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//---------------------------------------------------------------------------
// Include files
#include <file.h>
#include "gamelog.h"
#include <heap.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gameos.hpp>

bool isSetup = false;

//*****************************************************************************

GameLogPtr GameLog::files[MAX_GAMELOGS] = {NULL, NULL, NULL, NULL, NULL, NULL};

void* GameLog::operator new (size_t mySize) {

	void* result = NULL;
	
	result = gos_Malloc(mySize);
	
	return(result);
}

//---------------------------------------------------------------------------

void GameLog::operator delete (void* us) {

	gos_Free(us);
}

//---------------------------------------------------------------------------

void GameLog::dump (void) {

	//----------------
	// Dump to file...
	for (long i = 0; i < numLines; i++)
		filePtr->writeString(lines[i]);
	numLines = 0;
}

//---------------------------------------------------------------------------

void GameLog::close (void) {

	if (filePtr && inUse) {
		dump();
		char s[512];
		sprintf(s, "\nNum Total Lines = %ld\n", totalLines);
		filePtr->writeString(s);
		filePtr->close();
		inUse = false;
		numLines = 0;
		totalLines = 0;
	}
}

//---------------------------------------------------------------------------

void GameLog::destroy (void) {

	close();
}

//---------------------------------------------------------------------------

long GameLog::open (char* fileName) {

	numLines = 0;
	totalLines = 0;
	if (filePtr->create(fileName) != NO_ERR)
		return(-1);

	inUse = true;
	return(0);
}

//---------------------------------------------------------------------------

void GameLog::write (char* s) {

	static char buffer[MAX_GAMELOG_LINELEN];
	
	if (numLines == MAX_GAMELOG_LINES)
		dump();
	
	if (strlen(s) > (MAX_GAMELOG_LINELEN - 1))
		s[MAX_GAMELOG_LINELEN - 1] = NULL;

	sprintf(buffer, "%s\n", s);
	strncpy(lines[numLines], buffer, MAX_GAMELOG_LINELEN - 1);
	numLines++;
	totalLines++;
}

//---------------------------------------------------------------------------

GameLog* GameLog::getNewFile(void) { // MCHD CHANGE (02/17/15): Either fixed a bug or fucked something up

	if (!isSetup)
		setup();

	for (long i = 0; i < MAX_GAMELOGS; i++) 
		if (!files[i]->inUse) {
			return files[i];
		}
	return NULL;
}

//---------------------------------------------------------------------------

void GameLog::setup (void) {

	if (isSetup)
		return;

	isSetup = true;
	for (long i = 0; i < MAX_GAMELOGS; i++) {
		files[i] = new GameLog;
		files[i]->init();
		files[i]->handle = i;
		files[i]->inUse = false;
		files[i]->filePtr = new File;
		gosASSERT(files[i]->filePtr != NULL);
	}
}

//---------------------------------------------------------------------------

void GameLog::cleanup (void) {

	if (!isSetup)
		return;

	for (long i = 0; i < MAX_GAMELOGS; i++) 
	{
		if (files[i] && files[i]->inUse)
		{
			files[i]->close();
			files[i]->filePtr->close();
			delete files[i]->filePtr;
			files[i]->filePtr = NULL;
			delete files[i];
			files[i] = NULL;
		}
	}
}

