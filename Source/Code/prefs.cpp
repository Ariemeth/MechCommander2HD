//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include <io.h> // for _chmod()

#include "prefs.h"
#include "gamesound.h"
#include "MissionGui.h"

#include "soundSys.h"
#include "..\resource.h"

CPrefs CPrefs::originalSettings;

CPrefs::CPrefs() {
	DigitalMasterVolume = 255;
	MusicVolume = 64;
	sfxVolume = 64;
	RadioVolume = 64;
	BettyVolume = 64;

	useShadows = true;
	useTerrainDetailTexture = true;
	useHighObjectDetail = true;

	gameDifficulty = 0;
	unlimitedAmmo = true;

	renderer = 0;
	resolution = 1;
	fullScreen = false;
	gammaLevel = 0;
	leftRightMouseProfile = true; // if false, use old style commands
	for ( int i = 0; i < 10; i++ )
		playerName[i][0] = 0;

	for (i = 0; i < 10; i++ )
		ipAddresses[i][0] = 0;


	baseColor = 0xffff7e00;
	highlightColor = 0xffff7e00;
	faction = 0;
	insigniaFile[0] = 0;
	for ( i = 0; i < 10; i++ )
		unitName[i][0] = 0;
}

int CPrefs::load( const char* pFileName ) {
	long result = 0;

	FullPathFileName prefsPathname;
	prefsPathname.init("./",pFileName,".cfg");
	FitIniFilePtr prefsFile = new FitIniFile;

#ifdef _DEBUG
	long prefsOpenResult = 
#endif
	prefsFile->open(prefsPathname);

	gosASSERT (prefsOpenResult == NO_ERR);
	{
#ifdef _DEBUG
		long prefsBlockResult = 
#endif
			prefsFile->seekBlock("MechCommander2");
		gosASSERT(prefsBlockResult == NO_ERR);
		{
			// store volume settings in global variable since soundsystem 
			// does not exist yet.  These will be set in SoundSystem::init()
			result = prefsFile->readIdLong("DigitalMasterVolume",DigitalMasterVolume);
			if (result != NO_ERR)
				DigitalMasterVolume = 255;

			result = prefsFile->readIdLong("MusicVolume",MusicVolume);
			if (result != NO_ERR)
				MusicVolume = 64;

			result = prefsFile->readIdLong("RadioVolume",RadioVolume);
			if (result != NO_ERR)
				RadioVolume = 64;

			result = prefsFile->readIdLong("SFXVolume",sfxVolume);
			if (result != NO_ERR)
				sfxVolume = 64;

			result = prefsFile->readIdLong("BettyVolume",BettyVolume);
			if (result != NO_ERR)
				BettyVolume = 64;

			result = prefsFile->readIdBoolean( "Shadows", useShadows);
			if (result != NO_ERR)
				useShadows = true;

			result = prefsFile->readIdBoolean( "DetailTexture", useTerrainDetailTexture);
			if (result != NO_ERR)
				useTerrainDetailTexture = true;

			result = prefsFile->readIdBoolean( "HighObjectDetail", useHighObjectDetail );
			if ( result != NO_ERR )
				useHighObjectDetail = true;

			result = prefsFile->readIdLong("Difficulty",gameDifficulty);
			if (result != NO_ERR)
				gameDifficulty = 1;

			result = prefsFile->readIdBoolean("UnlimitedAmmo",unlimitedAmmo);
			if (result != NO_ERR)
				unlimitedAmmo = true;

			result = prefsFile->readIdLong("Rasterizer", renderer);
			if (result != NO_ERR)
				renderer = 0;

			if ((renderer < 0) || (renderer > 3))
				renderer = 0;

			// MCHD TODO:	Either detect the best renderer or maybe just hard-code it to zero.
			//				Just kill it after DX9 goes in.

			//Force use of video card which is above min spec.
			// Used ONLY if they are using a below minSpec Primary with an above minSpec secondary.
			if ((renderer >= 0) && (renderer < 3) &&
				(gos_GetMachineInformation(gos_Info_GetDeviceLocalMemory, renderer) +
				gos_GetMachineInformation(gos_Info_GetDeviceAGPMemory, renderer)) < 6291456)
			{
				long testRender = 0;
				while (testRender < 3)
				{
					if ((gos_GetMachineInformation(gos_Info_GetDeviceLocalMemory, testRender) +
						gos_GetMachineInformation(gos_Info_GetDeviceAGPMemory, testRender)) >= 6291456)
						break;

					testRender++;
				}

				if (testRender == 3)
				{
					//No video card in the machine above MinSpec.  Just set it to the first one.
					renderer = 0;
				}
			}
				
			result = prefsFile->readIdLong("Resolution",resolution);
			if (result != NO_ERR)
				resolution = 1;

			result = prefsFile->readIdBoolean("FullScreen",fullScreen);
			if (result != NO_ERR)
				fullScreen = false;

			result = prefsFile->readIdLong("Brightness",gammaLevel);
			if (result != NO_ERR)
				gammaLevel = 0;

			result = prefsFile->readIdBoolean( "UseLeftRightMouseProfile", leftRightMouseProfile );
			if ( result != NO_ERR )
				leftRightMouseProfile = true;

			char blockName[64];
			for ( int i = 0; i < 10; i++ )
			{	
				sprintf( blockName, "PlayerName%d", i );
				result = prefsFile->readIdString( blockName, &playerName[i][0], 255 );

				sprintf( blockName, "IPAddress%d", i );
				result = prefsFile->readIdString( blockName, &ipAddresses[i][0], 255 );

				sprintf( blockName, "UnitName%d", i );
				result = prefsFile->readIdString( blockName, &unitName[i][0], 255 );
			}

		
			result = prefsFile->readIdLong( "BaseColor", baseColor );
			if ( result != NO_ERR )	
				baseColor = 0xffff7e00;

			result = prefsFile->readIdLong( "Highlightcolor", highlightColor );
			if ( result != NO_ERR )	
				highlightColor = 0xffff7e00;
			
			result = prefsFile->readIdLong( "Faction", faction );
			result = prefsFile->readIdString( "InsigniaFile", insigniaFile, 255 );

			result = prefsFile->readIdBoolean( "PilotVideos",	pilotVideos );
			if ( result != NO_ERR )
			{
				pilotVideos = true;
			}
			result = prefsFile->readIdBoolean( "UseLocalShadows",	useLocalShadows );
			if ( result != NO_ERR )
			{
				useLocalShadows = true;
			}
			result = prefsFile->readIdBoolean( "UseNonWeaponEffects",	useNonWeaponEffects );
			if ( result != NO_ERR )
			{
				useNonWeaponEffects = 0;
			}
			result = prefsFile->readIdBoolean( "AsyncMouse",	asyncMouse );
			if ( result != NO_ERR )
				asyncMouse = 0;

			result = prefsFile->readIdLong( "FogPos",	fogPos );
			if ( result != NO_ERR )
			{
				fogPos = 50;
			}
			result = prefsFile->readIdChar( "BitDepth",	bitDepth );
			if ( result != NO_ERR )
				bitDepth = 0;

			if (bitDepth && (renderer == 3))
				bitDepth = 0;

			result = prefsFile->readIdBoolean( "SaveTranscripts", saveTranscripts );
			result = prefsFile->readIdBoolean( "Tutorials", tutorials );
			if ( result != NO_ERR )
				tutorials = true;

			// MCHD CHANGE (02/14/15): Moved the camera options into options.cfg
			useRealLOS = true; // MCHD TODO: Move this to debug options or something

			// Camera options
			result = prefsFile->readIdFloat("MaxClipDistance", Camera::maxClipDistance);
			if (result != NO_ERR)
				Camera::maxClipDistance = 3000.0f;

			result = prefsFile->readIdFloat("MinHazeDistance", Camera::minHazeDistance);
			if (result != NO_ERR)
				Camera::minHazeDistance = 2000.0f;

			result = prefsFile->readIdFloat("View0Zoom", Camera::cameraZoom[0]);
			if (result != NO_ERR)
				Camera::cameraZoom[0] = 1200.0f;

			result = prefsFile->readIdFloat("View0Tilt", Camera::cameraTilt[0]);
			if (result != NO_ERR)
				Camera::cameraTilt[0] = 35.0f;

			result = prefsFile->readIdFloat("View1Zoom", Camera::cameraZoom[1]);
			if (result != NO_ERR)
				Camera::cameraZoom[1] = 1200.0f;

			result = prefsFile->readIdFloat("View1Tilt", Camera::cameraTilt[1]);
			if (result != NO_ERR)
				Camera::cameraTilt[1] = 35.0f;

			result = prefsFile->readIdFloat("View2Zoom", Camera::cameraZoom[2]);
			if (result != NO_ERR)
				Camera::cameraZoom[2] = 1200.0f;

			result = prefsFile->readIdFloat("View2Tilt", Camera::cameraTilt[2]);
			if (result != NO_ERR)
				Camera::cameraTilt[2] = 35.0f;

			result = prefsFile->readIdFloat("View3Zoom", Camera::cameraZoom[3]);
			if (result != NO_ERR)
				Camera::cameraZoom[3] = 1200.0f;

			result = prefsFile->readIdFloat("View3Tilt", Camera::cameraTilt[3]);
			if (result != NO_ERR)
				Camera::cameraTilt[3] = 35.0f;
		}

	}
#ifndef VIEWER
	MissionInterfaceManager::loadHotKeys( *prefsFile);
#endif
	prefsFile->close();
	
	delete prefsFile;
	prefsFile = NULL;

	return 0;
}

int CPrefs::save() {
	char backupPath[256];
	char originalPath[256];
	
	strcpy( originalPath, "options.cfg" );
	strcpy( backupPath, originalPath );
	strcat( backupPath,".old" );
	_chmod(originalPath, _S_IWRITE);
	_chmod(backupPath, _S_IWRITE);
	remove(backupPath);
	rename(originalPath, backupPath);
	//MoveFileEx(originalPath, backupPath, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);

	FitIniFilePtr prefsFile = new FitIniFile;
	int result = prefsFile->create( (char*)originalPath );
	if (result != NO_ERR)
	{
		gosASSERT( false );
		return -1;
	}

	{
#ifdef _DEBUG
		long prefsBlockResult = 
#endif
		prefsFile->writeBlock("MechCommander2");
		gosASSERT(prefsBlockResult == (long)(strlen("\r\n[") + strlen("MechCommander2") + strlen("]\r\n")));
		{
			result = prefsFile->writeIdLong("DigitalMasterVolume",DigitalMasterVolume);
			result = prefsFile->writeIdLong("MusicVolume",MusicVolume);
			result = prefsFile->writeIdLong("RadioVolume",RadioVolume);
			result = prefsFile->writeIdLong("SFXVolume",sfxVolume);
			result = prefsFile->writeIdLong("BettyVolume",BettyVolume);

			result = prefsFile->writeIdBoolean( "Shadows", useShadows );
			result = prefsFile->writeIdBoolean( "DetailTexture", useTerrainDetailTexture );
			result = prefsFile->writeIdBoolean( "HighObjectDetail", useHighObjectDetail );

			result = prefsFile->writeIdLong("Difficulty",gameDifficulty);
			result = prefsFile->writeIdBoolean("UnlimitedAmmo",unlimitedAmmo);

			result = prefsFile->writeIdLong("Rasterizer",renderer);
			result = prefsFile->writeIdLong("Resolution",resolution);
			result = prefsFile->writeIdBoolean("FullScreen",fullScreen);
			result = prefsFile->writeIdLong("Brightness",gammaLevel);
			result = prefsFile->writeIdBoolean( "UseLeftRightMouseProfile", leftRightMouseProfile );
			char blockName[64];
			for ( int i = 0; i < 10; i++ )
			{	
				
				sprintf( blockName, "PlayerName%d", i );
				result = prefsFile->writeIdString( blockName, &playerName[i][0] );

				sprintf( blockName, "UnitName%d", i );
				result = prefsFile->writeIdString( blockName, &unitName[i][0] );

				sprintf( blockName, "IPAddress%d", i );
				result = prefsFile->writeIdString( blockName, &ipAddresses[i][0] );

			}

			result = prefsFile->writeIdLong( "BaseColor", baseColor );
			result = prefsFile->writeIdLong( "Highlightcolor", highlightColor );
			result = prefsFile->writeIdLong( "Faction", faction );
			result = prefsFile->writeIdString( "InsigniaFile", insigniaFile );

			result = prefsFile->writeIdBoolean( "PilotVideos",	pilotVideos );
			result = prefsFile->writeIdBoolean( "UseLocalShadows",	useLocalShadows );
			result = prefsFile->writeIdBoolean( "UseNonWeaponEffects",	useNonWeaponEffects );
			result = prefsFile->writeIdBoolean( "AsyncMouse",	asyncMouse );
			result = prefsFile->writeIdLong( "FogPos",	fogPos );
			result = prefsFile->writeIdChar( "BitDepth",	bitDepth );

			result = prefsFile->writeIdBoolean( "SaveTranscripts", saveTranscripts );
			result = prefsFile->writeIdBoolean( "Tutorials", tutorials );

			// MCHD CHANGE (02/14/15): Moved camera stuff into options.cfg
			result = prefsFile->writeIdFloat("MaxClipDistance", Camera::maxClipDistance);
			result = prefsFile->writeIdFloat("MinHazeDistance", Camera::minHazeDistance);

			result = prefsFile->writeIdFloat("View0Zoom", Camera::cameraZoom[0]);
			result = prefsFile->writeIdFloat("View0Tilt", Camera::cameraTilt[0]);

			result = prefsFile->writeIdFloat("View1Zoom", Camera::cameraZoom[1]);
			result = prefsFile->writeIdFloat("View1Tilt", Camera::cameraTilt[1]);

			result = prefsFile->writeIdFloat("View2Zoom", Camera::cameraZoom[2]);
			result = prefsFile->writeIdFloat("View2Tilt", Camera::cameraTilt[2]);

			result = prefsFile->writeIdFloat("View3Zoom", Camera::cameraZoom[3]);
			result = prefsFile->writeIdFloat("View3Tilt", Camera::cameraTilt[3]);
		}
	}
#ifndef VIEWER	
	MissionInterfaceManager::saveHotKeys( *prefsFile);
#endif
	prefsFile->close();
	
	
	delete prefsFile;
	prefsFile = NULL;

	return 0;
}

int CPrefs::applyPrefs(bool applyRes) {
	if (g_soundSystem) 
	{
		g_soundSystem->setDigitalMasterVolume(this->DigitalMasterVolume);
		g_soundSystem->setSFXVolume(this->sfxVolume);
		g_soundSystem->setRadioVolume(this->RadioVolume);
		g_soundSystem->setMusicVolume(this->MusicVolume);
		g_soundSystem->setBettyVolume(this->BettyVolume);
	}

	// MCHD CHANGE (02/18/15): Duplicate sound system removed

	// MCHD CHANGE (02/14/15): Fuck globals - it all lives in CPrefs now

	int bitDepth = this->bitDepth ? 32 : 16;

	//Play with the fog distance.
	float fogPercent = float(fogPos) / 100.0f;
	Camera::maxClipDistance = 3000.0f + (2000.0f * fogPercent);
	Camera::minHazeDistance	= 2000.0f + (2000.0f * fogPercent);

	gameVisibleVertices = 256; // MCHD CHANGE (02/14/15): Bumped this number up an arbitrary amount //30 + (30 * fogPercent);

	if (land)
	{
		land->resetVisibleVertices(gameVisibleVertices);
		land->update();
	}

	if ( applyRes )
	{
		long localRenderer = renderer;
		if ((renderer != 0) && (renderer != 3))
			localRenderer = 0;

		bool localFullScreen = fullScreen;
		bool localWindow = !fullScreen;
		if (Environment.fullScreen && fullScreen)
			localFullScreen = false;

		// MCHD TODO: Change resolution options
		switch (resolution)
		{
			case 0:			//640by480
				gos_GetMachineInformation( gos_Info_ValidMode, renderer, 640, 480, bitDepth);

				if (renderer == 3)
					gos_SetScreenMode(640,480,bitDepth,0,0,0,true,localFullScreen,0,localWindow,0,localRenderer);
				else
					gos_SetScreenMode(640,480,bitDepth,renderer,0,0,0,localFullScreen,0,localWindow,0,localRenderer);
				break;

			case 1:			//800by600
				gos_GetMachineInformation( gos_Info_ValidMode, renderer, 800, 600, bitDepth);

				if (renderer == 3)
					gos_SetScreenMode(800,600,bitDepth,0,0,0,true,localFullScreen,0,localWindow,0,localRenderer);
				else
					gos_SetScreenMode(800,600,bitDepth,renderer,0,0,0,localFullScreen,0,localWindow,0,localRenderer);
				break;

			case 2:			//1024by768
				gos_GetMachineInformation( gos_Info_ValidMode, renderer, 1024, 768, bitDepth);

				if (renderer == 3)
					gos_SetScreenMode(1024,768,bitDepth,0,0,0,true,localFullScreen,0,localWindow,0,localRenderer);
				else
					gos_SetScreenMode(1024,768,bitDepth,renderer,0,0,0,localFullScreen,0,localWindow,0,localRenderer);
				break;

			case 3:			//1280by1024
				gos_GetMachineInformation( gos_Info_ValidMode, renderer, 1280, 1024, bitDepth);

				if (renderer == 3)
					gos_SetScreenMode(1280,1024,bitDepth,0,0,0,true,localFullScreen,0,localWindow,0,localRenderer);
				else
					gos_SetScreenMode(1280,1024,bitDepth,renderer,0,0,0,localFullScreen,0,localWindow,0,localRenderer);
				break;

			case 4:			//1600by1200
				gos_GetMachineInformation( gos_Info_ValidMode, renderer, 1600, 1200, bitDepth);

				if (renderer == 3)
					gos_SetScreenMode(1600,1200,16,0,0,0,true,localFullScreen,0,localWindow,0,localRenderer);
				else
					gos_SetScreenMode(1600,1200,16,renderer,0,0,0,localFullScreen,0,localWindow,0,localRenderer);
				break;
		}
	}

	return 0;
}

void CPrefs::setNewName( const char* pNewName )
{
	if ( !pNewName )
		return;
	// check and see if this name is already in here
	for ( int i = 0; i < 10; i++ )
	{
		if ( !stricmp( pNewName, playerName[i] ) )
		{
			// found the same one so now we just shuffle
			for ( int j = i; j < 9; j++ )
			{
				strcpy( playerName[j], playerName[j+1] );
			}

			memmove( playerName[1], playerName[0], sizeof( char ) * 9 * 256 );
			strcpy( playerName[0], pNewName );

			break;
		}
		else if ( !strlen( playerName[i] ) )
		{
			// found the last one...

			memmove( playerName[1], playerName[0], sizeof( char ) * 9 * 256 );
			strcpy( playerName[0], pNewName );

			break;
		}
	}

	if ( i == 10 )
	{
		memmove( playerName[1], playerName[0], sizeof( char ) * 9 * 256 );
		strcpy( playerName[0], pNewName );
	}
}

void CPrefs::setNewIP( const char* pNewIP )
{
	if ( !pNewIP )
		return;
	// check and see if this name is already in here
	for ( int i = 0; i < 10; i++ )
	{
		if ( !stricmp( pNewIP, ipAddresses[i] ) )
		{
			// found the same one so now we just shuffle
			for ( int j = i; j < 9; j++ )
			{
				strcpy( ipAddresses[j], ipAddresses[j+1] );
			}

			memmove( ipAddresses[1], ipAddresses[0], sizeof( char ) * 9 * 24 );
			strcpy( ipAddresses[0], pNewIP );

			break;
		}
		else if ( !strlen( ipAddresses[i] ) )
		{
			// found the last one...

			memmove( ipAddresses[1], ipAddresses[0], sizeof( char ) * 9 * 24 );
			strcpy( ipAddresses[0], pNewIP );

			break;
		}
	}
}

void CPrefs::setNewUnit( const char* pNewUnit )
{
	if ( !pNewUnit )
		return;
// check and see if this name is already in here
	for ( int i = 0; i < 10; i++ )
	{
		if ( !stricmp( pNewUnit, unitName[i] ) )
		{
			// found the same one so now we just shuffle
			for ( int j = i; j < 9; j++ )
			{
				strcpy( unitName[j], unitName[j+1] );
			}

			memmove( unitName[1], unitName[0], sizeof( char ) * 9 * 256 );
			strcpy( unitName[0], pNewUnit );

			break;
		}
		else if ( !strlen( unitName[i] ) )
		{
			// found the last one...

			memmove( unitName[1], unitName[0], sizeof( char ) * 9 * 256 );
			strcpy( unitName[0], pNewUnit );

			break;
		}
	}

	if ( i == 10 )
	{
		memmove( unitName[1], unitName[0], sizeof( char ) * 9 * 256 );
		strcpy( unitName[0], pNewUnit );
	}
}



