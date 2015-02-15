//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef PREFS_H
#define PREFS_H

#ifndef MCLIB_H
#include "mclib.h"
#endif


class CPrefs {
public:
	static CPrefs originalSettings;

	CPrefs();
	int load( const char* pFileName = "options");
	int save();
	int applyPrefs(bool bApplyResolution = 1);

	void setNewName( const char* pNewName );
	void setNewUnit( const char* pNewUnit );
	void setNewIP( const char* pNewIP );

public:
	long DigitalMasterVolume;
	long MusicVolume;
	long sfxVolume;
	long RadioVolume;
	long BettyVolume;

	bool useShadows;
	bool useTerrainDetailTexture;
	bool useHighObjectDetail;

	long gameDifficulty;
	bool unlimitedAmmo;

	long renderer;
	long resolution;
	bool fullScreen;
	long gammaLevel;
	bool leftRightMouseProfile;
	long baseColor;
	long highlightColor;
	long faction;
	char insigniaFile[256];
	char unitName[10][256];
	char playerName[10][256];
	char ipAddresses[10][24];

	bool	pilotVideos;
	bool	useNonWeaponEffects;
	bool	useLocalShadows;
	bool	asyncMouse;
	bool	useRealLOS; // MCHD TODO: Investigate this and if we can use it for fog of war
	long	fogPos;
	long	gameVisibleVertices;
	char	bitDepth; // 0 == 16, 1 == 32

	bool	saveTranscripts;
	bool	tutorials;
};

extern CPrefs g_userPreferences;
#endif /*PREFS_H*/
