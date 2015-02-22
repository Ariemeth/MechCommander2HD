//--------------------------------------------------------------------------
// And away we go.  MechCommander 2 -- Electric Boogaloo
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//-----------------------------------
// Include Files
#include <GameOS.hpp>
#include <ToolOS.hpp>
#include <Stuff\stuff.hpp>
#include <MLR\MLR.hpp>
#include <GosFX\gosfxheaders.hpp>
#include "..\resource.h"
#include "mclib.h"
#include "mission.h"
#include "logistics.h"
#include "Soundsys.h"
#include "gamesound.h"
#include "multplyr.h"
#include "version.h"
#include "OptionsScreenWrapper.h"
#include "gamelog.h"
#include "LogisticsDialog.h"
#include "prefs.h"

//------------------------------------------------------------------------------------------------------------
// Command line
bool g_dbgGoalPlanEnemy = false;		// MCHD TODO: Still don't know what the hell goalPlan or enemyGoalPlan do
bool g_dbgShowMovers = false;			// '-show movers' forces the game to display all movers
long g_dbgResourcePoints = -1;		// '-rps [INT]' overrides the number of resource points for a mission
long g_dbgNumDisableAtStart = 0;				// '-disable [PARTID] [PARTID2] [ETC.]' disables up to MAX_KILL_AT_START objects at the start of a mission
#define	MAX_KILL_AT_START 100 
long g_dbgDisableAtStartIds[MAX_KILL_AT_START];
bool g_disableAmbientLight = false;			// '-killambient' sets ambient light to black
bool g_dbgLoadMission = false;			// '-mission ["MISSION_NAME"] launches straight into a mission
extern long g_dbgDrawCombatMove;				// '-debugcells [INT(1-4)]' draws a box around the combat move-to grid - TODO: Change to boolean

// Game
CPrefs g_userPreferences;
OptionsScreenWrapper * g_optionsScreenWrapper = NULL;	// MCHD TODO: Options or all menus?
bool g_gameStarted = false;
bool g_quitGame = false;
bool g_gamePaused = false;
bool g_invokeOptionsScreen = false;
bool g_EULAShown = false;

enum { DIFFICULTY_GREEN, DIFFICULTY_REGULAR, DIFFICULTY_VETERAN, DIFFICULTY_ELITE };	// MCHD CHANGE (02/02/15): Added enumeration for difficulty
long g_gameDifficulty = DIFFICULTY_GREEN;
char g_missionName[1024];
bool g_unlimitedAmmo = true;

extern char g_cdInstallPath[];			// MCHD TODO: Get rid of this
bool g_loadInMissionSave = false;			// MCHD TODO: What exactly does this do? It looks like it loads a test mission whenever load is clicked...
bool g_aborted = false;

// Rendering
enum { RENDERER_DEFAULT, RENDERER_PRIMARY, RENDERER_SECONDARY, RENDERER_SOFTWARE };		// MCHD CHANGE - 02/02/15 - Added enumeration for renderer; needs to change or go away
long g_renderer = RENDERER_DEFAULT;
long g_objectTextureSize = 128; // MCHD TODO: Move

extern float frameRate; // MCHD TODO: Get rid of this and calculate our own framerate
enum { CPU_UNKNOWN, CPU_PENTIUM, CPU_MMX, CPU_KATMAI } Processor = CPU_PENTIUM;		// MCHD TODO: Lol get rid of this shit

// Memory management
UserHeapPtr g_systemHeap = NULL;
UserHeapPtr g_GUIHeap = NULL;

#define KILOBYTE_TO_BYTES 1024
#define MEGABYTES_TO_BYTES (KILOBYTE_TO_BYTES * KILOBYTE_TO_BYTES)
unsigned long g_systemHeapSize = 256 * MEGABYTES_TO_BYTES;
unsigned long g_guiHeapSize = 4 * MEGABYTES_TO_BYTES;
unsigned long g_missionHeapSize = 16 * MEGABYTES_TO_BYTES;
unsigned long g_terrainHeapSize = 64 * MEGABYTES_TO_BYTES;
unsigned long g_spriteHeapSize = 64 * MEGABYTES_TO_BYTES;
unsigned long g_maxVertexCount = 1000000;

// Fast files
FastFile 	**g_fastFiles = NULL;
long 		g_numFastFiles = 0;
long		g_maxFastFiles = 0;

// Timers
TimerManagerPtr g_timerManager = NULL;

// Logging
GameLog* NetLog = NULL;
GameLog* CombatLog = NULL;
GameLog* BugLog = NULL;
bool initGameLogs = false;
bool initNetLog = false;
bool initCombatLog = false;
bool initBugLog = false;
bool initLRMoveLog = false;

extern char g_fileMissingString[];
extern char g_CDMissingString[];
extern char g_missingTitleString[];
char* g_ExceptionGameMsg = NULL;

// GameOS
DWORD gosResourceHandle = 0;
HGOSFONT3D gosFontHandle = 0;
float gosFontScale = 1.0;
FloatHelpPtr globalFloatHelp = NULL;
unsigned long currentFloatHelp = 0;

// MCHD TODO: Lol this is a joke. Get a realistic test and don't force a totally separate "hardware test" mode.
// Hardware detection
bool detectHardwareMode = false;
gos_VERTEX *testVertex = NULL;
WORD *indexArray = NULL;
DWORD testTextureHandle = 0xffffffff;
float totalTime = 0;
DWORD curDevice = 0;
#define MAX_HARDWARE_CARDS		5
float trisPerSecond[MAX_HARDWARE_CARDS] =
{
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f
};
extern DWORD NumDevices;
extern float ProcessorSpeed;
extern EString *g_textureCache_FilenameOfLastLoadedTexture;			// MCHD TODO: Check cache reads vs writes - then move it elsewhere

// Debug
#define	NUM_DEBUG_WINDOWS	4
char DebugStatusBarString[256];
GameDebugWindow* DebugWindow[NUM_DEBUG_WINDOWS] = { NULL, NULL, NULL, NULL };
GameObjectPtr DebugGameObject[3] = { NULL, NULL, NULL };
long GameObjectWindowList[3] = { 0, 0, 0 };
long NumGameObjectsToDisplay = 0;
bool DebugWindowOpen[NUM_DEBUG_WINDOWS] = { false, false, false, false };
bool DebugStatusBarOpen = false;
bool DebugScoreBoardOpen = false;
bool DebugHelpOpen = false;
void DEBUGWINS_print(char* s, long window = 0);

//---------------------------------------------------------------------------
#ifdef LAB_ONLY			// MCHD TODO: What is LAB_ONLY?
long currentLineElement = 0;
LineElement *debugLines[10000];

#define ProfileTime(x,y)	x=GetCycles();y;x=GetCycles()-x;
extern __int64 MCTimeMultiplayerUpdate;
#else
#define ProfileTime(x,y)	y;
#endif

// Forward declarations
void EnterWindowMode();
void InitDW(void);
void __stdcall ExitGameOS();

void detectHardwareInitialize();
void detectHardwareUpdate();
void detectHardwareRender();

void DEBUGWINS_init();
void DEBUGWINS_destroy();
void DEBUGWINS_update();
void DEBUGWINS_render();
void initDialogs();

char* GetGameInformation() 
{
	return(g_ExceptionGameMsg);
}

void UpdateGame()
{
	// Hardware detection mode - exit early
	if (detectHardwareMode)
	{
		detectHardwareUpdate();
		return;
	}

#ifdef LAB_ONLY		//Used for debugging LOS
	currentLineElement = 0;
#endif

	// If in multiplayer
	if (MPlayer)
	{
		ProfileTime(MCTimeMultiplayerUpdate, MPlayer->update());
		if (MPlayer->waitingToStartMission)
		{
			if (MPlayer->startMission)
			{
				g_soundSystem->playBettySample(BETTY_NEW_CAMPAIGN);
				MPlayer->waitingToStartMission = false;
			}
		}
	}

	// Reset floating tooltips
	FloatHelp::resetAll();

	//FrameRate can be zero in GameOS for some unknown fu-cocked reason.
	if (frameRate < Stuff::SMALL)
		frameRate = 4.0f;

	g_frameTime = 1.0f / frameRate;

	//-------------------------------------
	// MCHD TODO: Add a framerate counter

	//-------------------------------------
	// Poll devices for this frame.
	userInput->update();

	//----------------------------------------
	// Update the Sound System for this frame
	g_soundSystem->update();

	//----------------------------------------
	// Update all of the timers
	g_timerManager->update();

	//-----------------------------------------------------
	// Update Mission and Logistics here.
	if (logistics)
	{
		long result = logistics->update();
		if (result == log_DONE)
		{
			logistics->stop();
		}
	}

	// If the user is trying to open the options screen
	if ((true == g_invokeOptionsScreen)
		|| (userInput->getKeyDown(KEY_O) && userInput->ctrl() && !userInput->alt() && !userInput->shift()))
	{
		g_invokeOptionsScreen = false;
		if ((!g_optionsScreenWrapper) || (g_optionsScreenWrapper->isDone()))
		{
			if (!g_optionsScreenWrapper)
			{
				g_optionsScreenWrapper = new OptionsScreenWrapper;
				g_optionsScreenWrapper->init();
			}
			if (mission && mission->missionInterface && !mission->missionInterface->isPaused())
				mission->missionInterface->togglePauseWithoutMenu();
			g_optionsScreenWrapper->begin();
		}
	}
	// If a mission is happening
	else if (mission && (!g_optionsScreenWrapper || g_optionsScreenWrapper->isDone()))
	{
		// Update the mission and wait for it to end
		long result = mission->update();
		if (result == InvalidStatus)
		{
			mission->destroy();
			if (MPlayer && MPlayer->isServer())
				MPlayer->sendEndMission(1);
			g_quitGame = true;

			// MCHD TODO: Debug only?
			for (long i = 0; i < 3; i++)
			{
				DebugGameObject[i] = NULL;
				DebugWindow[i + 1]->clear();
			}
		}
		else if (result != mis_PLAYING)
		{
			//if (MPlayer && MPlayer->isServer())
			//	MPlayer->sendEndMission(1);
			if (result < mis_PLAYER_DRAW)
				g_soundSystem->playBettySample(BETTY_MISSION_LOST);
			else if (result > mis_PLAYER_DRAW)
				g_soundSystem->playBettySample(BETTY_MISSION_WON);

			//Gotta get rid of the mission textures before Heidi starts her stuff!
			// No RAM otherwise in mc2_18.
			mcTextureManager->flush(true);

			// If the player aborted a multiplayer match, close the session and return to the main screen
			if (MPlayer && g_aborted)
			{
				MPlayer->closeSession();
				MPlayer->setMode(MULTIPLAYER_MODE_NONE);
				logistics->start(log_SPLASH);
			}
			// Show the results otherwise
			else
			{
				logistics->start(log_RESULTS);
			}

			// Destroy the mission
			g_aborted = false;
			if (!LogisticsData::instance->campaignOver() || MPlayer || LogisticsData::instance->isSingleMission())
			{
				mission->destroy();
			}

			// MCHD TODO: Debug only?
			for (long i = 0; i < 3; i++)
			{
				DebugGameObject[i] = NULL;
				DebugWindow[i + 1]->clear();
			}
		}
	}
	// If the user is on a menu
	if (g_optionsScreenWrapper && !g_optionsScreenWrapper->isDone())
	{
		OptionsScreenWrapper::status_type result = g_optionsScreenWrapper->update();
		if (result == OptionsScreenWrapper::opt_DONE)
		{
			g_optionsScreenWrapper->end();
			if (mission && mission->missionInterface && mission->missionInterface->isPaused())
				mission->missionInterface->togglePauseWithoutMenu();
		}
	}

	// Update debug windows
	DEBUGWINS_update();

	//---------------------------------------------------
	// Update heap instrumentation.
#ifdef LAB_ONLY
	if (g_framesSinceMissionStart > 3)
		globalHeapList->update();
#endif

	//---------------------------------------------------------------
	// Quit
	if (g_quitGame)
	{
		gos_TerminateApplication();
	}

	// MCHD TODO: What?
	if (g_loadInMissionSave)
	{
		mission->load("data\\savegame\\testgame.ims");
		g_loadInMissionSave = false;
	}
}

void Render()
{
	// Detect hardware and bail out
	if (detectHardwareMode)
	{
		detectHardwareRender();
		return;
	}

	//---------------------------------------------------------------------------------
	// Set up basic render states
	DWORD bColor = 0x0;
	if (eye && mission->isActive())
		bColor = eye->fogColor;

	gos_SetupViewport(1, 1.0f, 1, bColor, 0.0f, 0.0f, 1.0f, 1.0f );		//ALWAYS FULL SCREEN for now
	gos_SetRenderState( gos_State_Filter, gos_FilterBiLinear );
	gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_AlphaInvAlpha );
	gos_SetRenderState( gos_State_AlphaTest, TRUE );
	gos_SetRenderState( gos_State_Clipping, TRUE);
	gos_SetRenderState( gos_State_TextureAddress, gos_TextureClamp );
	gos_SetRenderState( gos_State_Dither, 1);

	//---------------------------------------------------------------------------------------
	// 3D world
	if (mission && (!g_optionsScreenWrapper || g_optionsScreenWrapper->isDone()))
	{
		mission->render();
	}

	// Movies, pre-game, and post-game screens
	if (logistics)
	{
		float viewMulX, viewMulY, viewAddX, viewAddY;
		gos_GetViewport(&viewMulX, &viewMulY, &viewAddX, &viewAddY);
		userInput->setViewport(viewMulX,viewMulY,viewAddX,viewAddY);

		logistics->render();
	}

	// MCHD TODO: Options screen or is this all menus? I think it's all menus...
	if (g_optionsScreenWrapper && !g_optionsScreenWrapper->isDone() )
	{
		float viewMulX, viewMulY, viewAddX, viewAddY;
		gos_GetViewport(&viewMulX, &viewMulY, &viewAddX, &viewAddY);
		userInput->setViewport(viewMulX,viewMulY,viewAddX,viewAddY);

		g_optionsScreenWrapper->render();
	}

	//------------------------------------------------------------
	// Cursor
	gos_SetRenderState( gos_State_Filter, gos_FilterNone );
	userInput->render(); 

	// Debug windows
	DEBUGWINS_render();

	#ifdef LAB_ONLY
	if (currentLineElement)
	{
		for (long i=0;i<currentLineElement;i++)
		{
			debugLines[i]->draw();
		}
	}
	#endif
}

//---------------------------------------------------------------------------
#define GAME_REG_KEY	 	"Software\\Microsoft\\Microsoft Games\\MechCommander2\\1.0"
typedef DWORD (*EBUPROC) (LPCTSTR lpRegKeyLocation, LPCTSTR lpEULAFileName, LPCSTR lpWarrantyFileName, BOOL fCheckForFirstRun);

Stuff::MemoryStream *effectStream = NULL;
extern MidLevelRenderer::MLRClipper * theClipper;

void Initialize()
{
	__asm push esi; // MCHD TODO: What?

	// gotta have this no matter what
	gosResourceHandle = gos_OpenResourceDLL("mc2res.dll");

	//Check for enough SwapFile Space FIRST!!!
	// In order to do that, we must force Win2K/XP to enlarge
	// its swapfile at the get go to insure goodness and that
	// the message does not come up during game run.
	void *testMem = VirtualAlloc(NULL,123000000,MEM_COMMIT,PAGE_READWRITE);

   	MEMORYSTATUS ms;
   	GlobalMemoryStatus( &ms );
   	if (ms.dwTotalPageFile < 250000000)
   	{
   		char txt[4096];
   		char msg[4096];
   		cLoadString(IDS_SWAPFILE_TOO_SMALL,txt,4095);
   		sprintf(msg,txt,(ms.dwAvailPageFile / (1024 * 1024)));

   		char caption[1024];
   		cLoadString(IDS_SWAPFILE_CAPTION,caption,1023);

   		MessageBox(NULL,msg,caption,MB_OK | MB_ICONWARNING);

   		gos_TerminateApplication();
   		return;
   	}

	if (testMem)
		VirtualFree(testMem,0,MEM_RELEASE);

	cLoadString(IDS_MC2_FILEMISSING,g_fileMissingString,511);
	cLoadString(IDS_MC2_CDMISSING,g_CDMissingString,1023);
	cLoadString(IDS_MC2_MISSING_TITLE,g_missingTitleString,255);

	//Check for sufficient hard Drive space on drive game is running from
	char currentPath[1024];
	gos_GetCurrentPath( currentPath, 1023 );
	__int64 driveSpace = gos_GetDriveFreeSpace(currentPath);
	if (driveSpace < (20 * 1024 * 1024))
	{
		char title[256];
		char msg[2048];
		cLoadString(IDS_GAME_HDSPACE_ERROR,title,255);
		cLoadString(IDS_GAME_HDSPACE_MSG,msg,2047);
		DWORD result = MessageBox(NULL,msg,title,MB_OKCANCEL | MB_ICONWARNING);
		if (result == IDCANCEL)
			ExitGameOS();
	}

	//Check if we are a Voodoo 3.  If so, ONLY allow editor to run IF
	// Starting resolution is 1024x768x16 or LOWER.  NO 32-BIT ALLOWED!
	if ((gos_GetMachineInformation(gos_Info_GetDeviceVendorID,0) == 0x121a) &&
		(gos_GetMachineInformation(gos_Info_GetDeviceDeviceID,0) == 0x0005))
	{
		DEVMODE dev;
		memset( &dev, 0, sizeof ( DEVMODE ) );
		dev.dmSize = sizeof( DEVMODE );
		dev.dmSpecVersion = DM_SPECVERSION;
		EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &dev );

		if ((dev.dmPelsWidth > 1024) || (dev.dmPelsHeight > 768) || (dev.dmBitsPerPel > 16))
		{
			char title[256];
			char msg[2048];
			cLoadString(IDS_GAME_ERROR,title,255);
			cLoadString(IDS_GAME_VOODOO3,msg,2047);
			MessageBox(NULL,msg,title,MB_OK | MB_ICONWARNING);
			ExitGameOS();
		}
	}

	//Then, we should check to see if the options.cfg exists.  if NOT, 
	// Bring up a sniffer warning dialog, sniff, bring up sniffer end dialog and end.
	if (!fileExists("options.cfg"))
	{
		//Must also check for a RIVA TNT card with driver 4.10.1.0131
		// If this card is in the machine with this driver, do NOT allow sniffer to run.
		// Just copy minprefs.cfg to options.cfg and move on.
		if ((gos_GetMachineInformation(gos_Info_GetDeviceVendorID,0) == 0x10de) &&
			(gos_GetMachineInformation(gos_Info_GetDeviceDeviceID,0) == 0x0020) &&
			(gos_GetMachineInformation(gos_Info_GetDeviceDriverVersionHi,0) == 0x0004000a) &&
			(gos_GetMachineInformation(gos_Info_GetDeviceDriverVersionLo,0) == 0x00010083))
		{
			CopyFile("minprefs.cfg","orgprefs.cfg",true);
			CopyFile("minprefs.cfg","options.cfg",true);
		}
		else
		{
			//NEW Plan.  Regardless, copy the minprefs to options.  That way, sniffer can only screw up once!!
			// After the first run, it will no longer run and the game will continue with minprefs.
			detectHardwareMode = true;
			CopyFile("minprefs.cfg","orgprefs.cfg",true);
			CopyFile("minprefs.cfg","options.cfg",true);
		}
	}

	//Seed the random Number generator.
	gos_srand(timeGetTime());

	// If the game is running hardware detection, set up the test and exit early
	if (detectHardwareMode)
	{
		detectHardwareInitialize();

		//Make any directories we need which should be empty.
		CreateDirectory(savePath, NULL);
		CreateDirectory(transcriptsPath, NULL);

		//Startup the Office Watson Handler.
		InitDW();

		__asm pop esi;
		return;
	}

	//---------------------------------------------------------------------
	// Initialize the game
	Environment.Key_Exit=-1; // so escape doesn't kill your app
	
	//-------------------------------------------------------------
	// Find the CDPath in the registry and save it off so I can
	// look in CD Install Path for files.
		
	//Changed for the shared source release, just set to current directory
	//DWORD maxPathLength = 1023;
	//gos_LoadDataFromRegistry("CDPath", g_cdInstallPath, &maxPathLength);
	//if (!maxPathLength)
	//	strcpy(g_cdInstallPath,"..\\");
	strcpy(g_cdInstallPath,".\\");

	//--------------------------------------------------------------
	// Start the SystemHeap and globalHeapList
	globalHeapList = new HeapList;
	gosASSERT(globalHeapList != NULL);
	
	globalHeapList->init();
	globalHeapList->update();		//Run Instrumentation into GOS Debugger Screen
	
	g_systemHeap = new UserHeap;
	gosASSERT(g_systemHeap != NULL);
	
	g_systemHeap->init(g_systemHeapSize,"SYSTEM");
	
	//Check for OrgPrefs.cfg.  If this file is not present,
	// We need to create it because sniffer didn't run.
	// Create a new prefs.cfg, too.  This way, we can easily restore
	// them to a running state if they screw up their prefs.
	// All they have to do is delete the orgprefs.cfg file.
	// OR re-install!!
	// ALWAYS use minPrefs.cfg here!
	if (!fileExists("orgprefs.cfg"))
	{
		CopyFile("minprefs.cfg","orgprefs.cfg",true);
		CopyFile("minprefs.cfg","options.cfg",true);
	}

	//--------------------------------------------------------------
	// Read in System.CFG
	FitIniFilePtr systemFile = new FitIniFile;
	
#ifdef _DEBUG
	long systemOpenResult = 
#endif
		systemFile->open("system.cfg");
			   
#ifdef _DEBUG
	if( systemOpenResult != NO_ERR)
	{
		char Buffer[256];
		gos_GetCurrentPath( Buffer, 256 );
		STOP(( "Cannot find \"system.cfg\" file in %s",Buffer ));
	}
#endif
	
	{
		// MCHD CHANGE (02/14/15): Not reading in heap size variables anymore
	
#ifdef _DEBUG
		long systemPathResult = 
#endif
			systemFile->seekBlock("systemPaths");
		gosASSERT(systemPathResult == NO_ERR);
		{
			long result = systemFile->readIdString("terrainPath",terrainPath,79);
			gosASSERT(result == NO_ERR);
	
			result = systemFile->readIdString("artPath",artPath,79);
			gosASSERT(result == NO_ERR);
	
			result = systemFile->readIdString("fontPath",fontPath,79);
			gosASSERT(result == NO_ERR);
	
			result = systemFile->readIdString("savePath",savePath,79);
			gosASSERT(result == NO_ERR);
	
			result = systemFile->readIdString("spritePath",spritePath,79);
			gosASSERT(result == NO_ERR);
	
			result = systemFile->readIdString("shapesPath",shapesPath,79);
			gosASSERT(result == NO_ERR);
	
			result = systemFile->readIdString("soundPath",soundPath,79);
			gosASSERT(result == NO_ERR);
	
			result = systemFile->readIdString("objectPath",objectPath,79);
			gosASSERT(result == NO_ERR);
	
			result = systemFile->readIdString("cameraPath",cameraPath,79);
			gosASSERT(result == NO_ERR);
	
			result = systemFile->readIdString("tilePath",tilePath,79);
			gosASSERT(result == NO_ERR);
	
			result = systemFile->readIdString("missionPath",missionPath,79);
			gosASSERT(result == NO_ERR);
	
			result = systemFile->readIdString("warriorPath",warriorPath,79);
			gosASSERT(result == NO_ERR);
	
			result = systemFile->readIdString("profilePath",profilePath,79);
			gosASSERT(result == NO_ERR);
	
			result = systemFile->readIdString("interfacepath",interfacePath,79);
			gosASSERT(result == NO_ERR);
	
			result = systemFile->readIdString("moviepath",moviePath,79);
			gosASSERT(result == NO_ERR);
	
			result = systemFile->readIdString("CDsoundPath",CDsoundPath,79);
			gosASSERT(result == NO_ERR);
	
			result = systemFile->readIdString("CDmoviepath",CDmoviePath,79);
			gosASSERT(result == NO_ERR);
	
			result = systemFile->readIdString("CDspritePath",CDspritePath,79);
			gosASSERT(result == NO_ERR);
		}
	
#ifdef _DEBUG
		long fastFileResult = 
#endif
		systemFile->seekBlock("FastFiles");
		gosASSERT(fastFileResult == NO_ERR);
		{
			long result = systemFile->readIdLong("NumFastFiles",g_maxFastFiles);
			if (result != NO_ERR)
				g_maxFastFiles = 0;
	
			if (g_maxFastFiles)
			{
				g_fastFiles = (FastFile **)malloc(g_maxFastFiles*sizeof(FastFile *));
				memset(g_fastFiles,0,g_maxFastFiles*sizeof(FastFile *));
	
				long fileNum = 0;
				char fastFileId[10];
				char fileName[100];
				sprintf(fastFileId,"File%ld",fileNum);
		
				while (systemFile->readIdString(fastFileId,fileName,99) == NO_ERR)
				{
					bool result = FastFileInit(fileName);
					if (!result)
						STOP(("Unable to startup fastfiles.  Probably an old one in the directory!!"));
	
					fileNum++;
					sprintf(fastFileId,"File%ld",fileNum);
				}
			}
		}
			
		// MCHD CHANGE (02/14/15): Always use sound and music - user can set in options

		long result = systemFile->seekBlock("CameraSettings");
		if (result == NO_ERR)
		{
			result = systemFile->readIdFloat("MaxPerspective",Camera::MAX_PERSPECTIVE);
			if (result != NO_ERR)
				Camera::MAX_PERSPECTIVE = 88.0f;

			if (Camera::MAX_PERSPECTIVE > 90.0f)
				Camera::MAX_PERSPECTIVE = 90.0f;

			result = systemFile->readIdFloat("MinPerspective",Camera::MIN_PERSPECTIVE);
			if (result != NO_ERR)
				Camera::MIN_PERSPECTIVE = 18.0f;

			if (Camera::MIN_PERSPECTIVE < 0.0f)
				Camera::MIN_PERSPECTIVE = 0.0f;

			result = systemFile->readIdFloat("MaxOrtho",Camera::MAX_ORTHO);
			if (result != NO_ERR)
				Camera::MAX_ORTHO = 88.0f;

			if (Camera::MAX_ORTHO > 90.0f)
				Camera::MAX_ORTHO = 90.0f;

			result = systemFile->readIdFloat("MinOrtho",Camera::MIN_ORTHO);
			if (result != NO_ERR)
				Camera::MIN_ORTHO = 18.0f;

			if (Camera::MIN_ORTHO < 0.0f)
				Camera::MIN_ORTHO = 0.0f;

			result = systemFile->readIdFloat("AltitudeMinimum",Camera::AltitudeMinimum);
			if (result != NO_ERR)
				Camera::AltitudeMinimum = 560.0f;

			if (Camera::AltitudeMinimum < 110.0f)
				Camera::AltitudeMinimum = 110.0f;

			result = systemFile->readIdFloat("AltitudeMaximumHi",Camera::AltitudeMaximumHi);
			if (result != NO_ERR)
				Camera::AltitudeMaximumHi = 1600.0f;

			result = systemFile->readIdFloat("AltitudeMaximumLo",Camera::AltitudeMaximumLo);
			if (result != NO_ERR)
				Camera::AltitudeMaximumHi = 1500.0f;
		}
	}
	
	systemFile->close();
	delete systemFile;
	systemFile = NULL;
	
	if (initGameLogs) {
		GameLog::setup();
		if (initNetLog && !NetLog) {
			NetLog = GameLog::getNewFile();
			if (!NetLog)
				Fatal(0, " Couldn't create Net Log ");
			long err = NetLog->open("net.log");
			if (err)
				Fatal(0, " Couldn't open Net Log ");
		}
		if (initCombatLog && !CombatLog) {
			CombatLog = GameLog::getNewFile();
			if (!CombatLog)
				Fatal(0, " Couldn't create Combat Log ");
			long err = CombatLog->open("combat.log");
			if (err)
				Fatal(0, " Couldn't open Combat Log ");
		}
		if (initBugLog && !BugLog) {
			BugLog = GameLog::getNewFile();
			if (!BugLog)
				Fatal(0, " Couldn't create Bug Log ");
			long err = BugLog->open("bug.log");
			if (err)
				Fatal(0, " Couldn't open Bug Log ");
		}
		if (initLRMoveLog && !GlobalMap::logEnabled) {
			GlobalMap::toggleLog();
		}
	}
	
	//--------------------------------------------------------------
	// MCHD CHANGE (02/14/15): Redundant option loading block deleted - everything moved to CPrefs::load()

	// Load and apply options from "options.cfg"
	g_userPreferences.load();
	
	//--------------------------------------------------
	// Setup Mouse Parameter
	float doubleClickThreshold = 0.2f;
	long dragThreshold = 0.016667f;
	userInput = new UserInput;
	userInput->init();
	userInput->setMouseDoubleClickThreshold(doubleClickThreshold);
	userInput->setMouseDragThreshold(dragThreshold);

	//--------------------------------------------------
	// Tooltips
	char temp[256];
	cLoadString( IDS_FLOAT_HELP_FONT, temp, 255 );
	char* pStr = strstr( temp, "," );
	if ( pStr )
	{
		gosFontScale = -atoi( pStr + 1 );
		*pStr = 0;
	}
	char path [256];
	strcpy( path, "assets\\graphics\\" );
	strcat( path, temp );	
	
	gosFontHandle = gos_LoadFont(path);
	globalFloatHelp = new FloatHelp(MAX_FLOAT_HELPS);
	
	//----------------------------------
	// Start other modules
	//----------------------------------
		
	Stuff::InitializeClasses();
	MidLevelRenderer::InitializeClasses(8192*4,8192,0,0,true);
	gosFX::InitializeClasses();
		
	gos_PushCurrentHeap(MidLevelRenderer::Heap);

	MidLevelRenderer::TGAFilePool *pool = new MidLevelRenderer::TGAFilePool("data\\tgl\\128\\");
	MidLevelRenderer::MLRTexturePool::Instance = new MidLevelRenderer::MLRTexturePool(pool);
	MidLevelRenderer::MLRSortByOrder *cameraSorter = new MidLevelRenderer::MLRSortByOrder(MidLevelRenderer::MLRTexturePool::Instance);
	theClipper = new MidLevelRenderer::MLRClipper(0, cameraSorter);

	gos_PopCurrentHeap();
	
	//------------------------------------------------------
	// Start the GOS FX.
	gos_PushCurrentHeap(gosFX::Heap);

	gosFX::EffectLibrary::Instance = new gosFX::EffectLibrary();
	Check_Object(gosFX::EffectLibrary::Instance);

	FullPathFileName effectsName;
	effectsName.init(effectsPath, "mc2.fx", "");

	File effectFile;
	long result = effectFile.open(effectsName);
	if (result != NO_ERR)
		STOP(("Could not find MC2.fx"));

	long effectsSize = effectFile.fileSize();
	MemoryPtr effectsData = (MemoryPtr)g_systemHeap->Malloc(effectsSize);
	effectFile.read(effectsData, effectsSize);
	effectFile.close();

	effectStream = new Stuff::MemoryStream(effectsData, effectsSize);
	gosFX::EffectLibrary::Instance->Load(effectStream);

	gosFX::LightManager::Instance = new gosFX::LightManager();

	gos_PopCurrentHeap();
	g_systemHeap->Free(effectsData);
	
	//--------------------------------------------------------------
	// Start the GUI Heap.
	g_GUIHeap = new UserHeap;
	gosASSERT(g_GUIHeap != NULL);
	g_GUIHeap->init(g_guiHeapSize,"GUI");
		
	//------------------------------------------------
	// Fire up the MC Texture Manager.
	mcTextureManager = new MC_TextureManager;
	mcTextureManager->start();
	
	//--------------------------------------------------------------
	// Load up the mouse cursors
	userInput->initMouseCursors("cursors");
	userInput->mouseOff();
	userInput->setMouseCursor(mState_NORMAL);
	
	//------------------------------------------------
	// Give the Sound System a Whirl!
	g_soundSystem = new GameSoundSystem;
	g_soundSystem->init();
	((SoundSystem *)g_soundSystem)->init("sound");

	g_userPreferences.applyPrefs(); // MCHD CHANGE (02/18/15): Moved to after sound initialization because volume
		
	//-----------------------------------------------
	// Only used by camera to retrieve screen coords.
	globalPane = new PANE;
	globalWindow = new WINDOW;
	
	globalPane->window = globalWindow;
	globalPane->x0 = 0;
	globalPane->y0 = 0;
	globalPane->x1 = Environment.screenWidth;
	globalPane->y1 = Environment.screenHeight;
	
	globalWindow->buffer = NULL;			//This is set at the start of Renders.  For now we HOLD LOCK during entire old 2D render test.  This will go away once we hit 3D
	globalWindow->shadow = NULL;
	globalWindow->stencil = NULL;
	globalWindow->x_max = globalPane->x1 - globalPane->x0 - 1;
	globalWindow->y_max = globalPane->y1 - globalPane->y0 - 1;
	
	//---------------------------------------------------------
	// Start the Timers
	g_timerManager = new TimerManager;
	g_timerManager->init();
	
	//---------------------------------------------------------
	// Start the Color table code
	initColorTables();
				
	//---------------------------------------------------------
	// Missions, movies, and pre/post-round stuff
	mission = new Mission;
	logistics = new Logistics;
	
	// Debug
	GameDebugWindow::setFont("assets\\graphics\\arial8.tga");
	DEBUGWINS_init();
	
	// Networking
	StartupNetworking();

	// If the command line specified to launch directly into a mission
	if (g_dbgLoadMission)
	{
		logistics->setLogisticsState(log_STARTMISSIONFROMCMDLINE);
		char commandersToLoad[MAX_MC_PLAYERS][3] = {{0, 0, 0}, {1, 1, 1}, {2, 0, 2}, {3, 3, 3}, {4, 4, 4}, {5, 5, 5}, {6, 6, 6}, {7, 7, 7}};
		mission->init(g_missionName, MISSION_LOAD_SP_QUICKSTART, 0, NULL, commandersToLoad, 2);
		eye->activate();
		eye->update();
		mission->start();
	}
	// Otherwise, launch the game like normal
	else
	{
		int param = log_SPLASH;
		if ( MPlayer && MPlayer->launchedFromLobby )
			param = log_ZONE;
		logistics->start(param);
		Mission::initBareMinimum();
	}
	
	// MCHD TODO: What is it?
	initDialogs();
	
	gos_EnableSetting(gos_Set_LoseFocusBehavior, 2 );
	DWORD numJoysticks = gosJoystick_CountJoysticks();
	for (long i = 0; i < numJoysticks; i++)
	{
		gosJoystick_Info joyInfo;
		gosJoystick_GetInfo(i, &joyInfo);
			
		//Search for the Attila Strategic Commander here.
		// Look for the Attila VidPid 0x045e0033 (Unique!)
		// Andy G did not do what I asked for here.  Will do in our version when we branch.
		// For NOW, ANY 3 Axis joystick with 15 buttons is a Strategic Commander.
		if ((joyInfo.nAxes == 3) && (joyInfo.nButtons == 15))
		{
			userInput->addAttila(i);
			gosJoystick_SetPolling(i, true, 0.1f);
		}
	}
	
	// MCHD TODO: Uhhh... what? Can we just get rid of this shit?
	//Time BOMB goes here.
	// Set Date and write Binary data to registry under key
	// GraphicsData!!
	SYSTEMTIME bombDate;
	DWORD dataSize = sizeof(SYSTEMTIME);
	gos_LoadDataFromRegistry("GraphicsDataInit2", &bombDate, &dataSize);
	if (dataSize == 0)
	{
		bombDate.wYear = 2001;
		bombDate.wMonth = 6;
		bombDate.wDayOfWeek = 5;
		bombDate.wDay = 15;
		bombDate.wHour = 0;
		bombDate.wMinute = 0;
		bombDate.wSecond = 0;
		bombDate.wMilliseconds = 0;
		
		dataSize = sizeof(SYSTEMTIME);
		gos_SaveDataToRegistry("GraphicsDataInit2", &bombDate, dataSize);
	}

	// Mark the game as fully initialized
	g_gameStarted = true;

	//Make any directories we need which should be empty.
	CreateDirectory(savePath,NULL);
	CreateDirectory(transcriptsPath,NULL);

	//Startup the Office Watson Handler.
	InitDW();

	__asm pop esi;
}

//---------------------------------------------------------------------------

void Terminate()
{
	// If the game hasn't fully initialized, don't try to delete things
	if (!g_gameStarted)
		return;

	// If we're running hardware detection, delete the test arrays and bail out early
	if (detectHardwareMode)
	{
		free(testVertex);
		testVertex = NULL;
		free(indexArray);
		indexArray = NULL;
		return;
	}

	//--------------------------------------------------
	// Setup Mouse Parameters from Prefs.CFG
	if (userInput)
	{
		//Must call this to shut off the mouse async draw thread.
		//DO NOT DELETE Yet!!  Too many things assume this is around.
		userInput->destroy();
	}

	//--------------------------------------------------

	// Debug
	DEBUGWINS_destroy();

	// Networking
	ShutdownNetworking();

	// Clipping mechanism
	delete theClipper;
	theClipper = NULL;

	//---------------------------------------------------------
	// Mission, movies, and pre/post-game
	if (mission)
	{
		mission->destroy(false);
		delete mission;
		mission = NULL;
	}
	if (logistics)
	{
		logistics->destroy();
		delete logistics;
		logistics = NULL;
	}

	endDialogs();

	// Menus
	if (g_optionsScreenWrapper)
	{
		g_optionsScreenWrapper->destroy();
		delete g_optionsScreenWrapper;
		g_optionsScreenWrapper = NULL;
	}

	//---------------------------------------------------------
	// Start the Color table code
	destroyColorTables();

	// Logging
	GameLog::cleanup();

	//---------------------------------------------------------
	// End the Timers
	delete g_timerManager;
	g_timerManager = NULL;

	//---------------------------------------------------------
	// TEST of PORT
	// Create VFX PANE and WINDOW to test draw of old terrain!
	if (globalPane)
	{
		delete globalPane;
		globalPane = NULL;
	}
	if (globalWindow)
	{
		delete globalWindow;
		globalWindow = NULL;
	}

	//-------------------------------------------------------------
	// Shut down the soundSytem for a change!
	if (g_soundSystem)
	{
		g_soundSystem->destroy();
		delete g_soundSystem;
		g_soundSystem = NULL;
	}

	//------------------------------------------------
	// shutdown the MC Texture Manager.
	if (mcTextureManager)
	{
		mcTextureManager->destroy();
		delete mcTextureManager;
		mcTextureManager = NULL;
	}

	// Tooltips
	delete globalFloatHelp;
	globalFloatHelp = NULL;

	//--------------------------------------------------------------
	// Memory
	if (g_systemHeap)
	{
		g_systemHeap->destroy();
		delete g_systemHeap;
		g_systemHeap = NULL;
	}
	if (g_GUIHeap)
	{
		g_GUIHeap->destroy();
		delete g_GUIHeap;
		g_GUIHeap = NULL;
	}
	if (globalHeapList)
	{
		globalHeapList->destroy();
		delete globalHeapList;
		globalHeapList = NULL;
	}

	//----------------------------------------------------
	// Shutdown the MLR and associated stuff libraries
	//----------------------------------------------------
	gos_PushCurrentHeap(gosFX::Heap);

	// Free memory
	delete effectStream;
	delete gosFX::LightManager::Instance;

	gos_PopCurrentHeap();

	// Turn off libraries
	gosFX::TerminateClasses();
	MidLevelRenderer::TerminateClasses();
	Stuff::TerminateClasses();

	// User input
	delete userInput;
	userInput = NULL;

	// GameOS
	gos_CloseResourceDLL(gosResourceHandle);

	// Turn off the fast Files
	FastFileFini();

	// MCHD TODO: Get this the hell out of here --> static class functions
	// Clean up any global allocations
	if (g_textureCache_FilenameOfLastLoadedTexture)
	{
		delete g_textureCache_FilenameOfLastLoadedTexture;
		g_textureCache_FilenameOfLastLoadedTexture = NULL;
	}
	if (ForceGroupIcon::jumpJetIcon)
	{
		delete ForceGroupIcon::jumpJetIcon;
		ForceGroupIcon::jumpJetIcon = NULL;
	}
	if (MechIcon::s_MechTextures)
	{
		delete[] MechIcon::s_MechTextures;
		MechIcon::s_MechTextures = NULL;
	}
	if (ForceGroupIcon::s_textureMemory)
	{
		delete[] ForceGroupIcon::s_textureMemory;
		ForceGroupIcon::s_textureMemory = NULL;
	}
}

//---------------------------------------------------------------------------

void detectHardwareUpdate()
{
	static DWORD curIteration = 0;
	static bool detectionComplete = false;
	if (!detectionComplete)
	{
		for (long i = 0; i<3000; i++)
		{
			testVertex[i].x = RandomNumber(1000) - 100;
			testVertex[i].y = RandomNumber(800) - 100;
			testVertex[i].z = ((float)RandomNumber(150) / 100.0f) - 0.25f;
			testVertex[i].rhw = 0.5f;
			testVertex[i].u = (float)RandomNumber(100) / 100.0f;
			testVertex[i].v = (float)RandomNumber(100) / 100.0f;
			testVertex[i].argb = 0x3fffffff;
			testVertex[i].frgb = 0x3f1f2f3f;
			indexArray[i] = i;
		}

		curIteration++;
#define NUM_ITERATIONS 4
		if (curIteration > NUM_ITERATIONS)
		{
			if (gos_GetMachineInformation(gos_Info_GetDeviceLocalMemory, curDevice) >= 6291456)
				trisPerSecond[curDevice] = 5000.0f * NUM_ITERATIONS / (totalTime / 1000);
			else
				trisPerSecond[curDevice] = 0.0f;

			curDevice++;
			if ((curDevice >= NumDevices) || (curDevice > MAX_HARDWARE_CARDS))
			{
				FILE *sniffData = fopen("sniff.dat", "wt");
				if (sniffData)
				{
					char sniff[2048];
					sprintf(sniff, "%f,%f,%f,%f,%f,%f\n", trisPerSecond[0], ProcessorSpeed, trisPerSecond[1], trisPerSecond[2], trisPerSecond[3], trisPerSecond[4]);

					fputs(sniff, sniffData);
					fclose(sniffData);
				}

				if ((trisPerSecond[0] > 1000000.0f) && (ProcessorSpeed > 595.0f))
				{
					CopyFile("hiPrefs.cfg", "orgprefs.cfg", false);
					CopyFile("hiPrefs.cfg", "options.cfg", false);
				}
				else if ((trisPerSecond[0] > 1000000.0f) && (ProcessorSpeed <= 595.0f))
				{
					CopyFile("h2Prefs.cfg", "orgprefs.cfg", false);
					CopyFile("h2Prefs.cfg", "options.cfg", false);
				}
				else if ((trisPerSecond[0] > 10000.0f) && (ProcessorSpeed > 595.0f))
				{
					CopyFile("midPrefs.cfg", "orgprefs.cfg", false);
					CopyFile("midPrefs.cfg", "options.cfg", false);
				}
				else if ((trisPerSecond[1] > 1000000.0f) && (ProcessorSpeed > 595.0f))
				{
					CopyFile("hiPrefs1.cfg", "orgprefs.cfg", false);
					CopyFile("hiPrefs1.cfg", "options.cfg", false);
				}
				else if ((trisPerSecond[1] > 1000000.0f) && (ProcessorSpeed <= 595.0f))
				{
					CopyFile("h2Prefs1.cfg", "orgprefs.cfg", false);
					CopyFile("h2Prefs1.cfg", "options.cfg", false);
				}
				else if ((trisPerSecond[1] > 10000.0f) && (ProcessorSpeed > 595.0f))
				{
					CopyFile("midPrefs1.cfg", "orgprefs.cfg", false);
					CopyFile("midPrefs1.cfg", "options.cfg", false);
				}
				else if (trisPerSecond[1] > 0.0f)
				{
					CopyFile("minPrefs1.cfg", "orgprefs.cfg", false);
					CopyFile("minPrefs1.cfg", "options.cfg", false);
				}
				else
				{
					CopyFile("minPrefs.cfg", "orgprefs.cfg", false);
					CopyFile("minPrefs.cfg", "options.cfg", false);
				}

				detectionComplete = true;
			}
			else
			{
				//Don't sniff a below minspec card.  Crash O Roni!
				if (gos_GetMachineInformation(gos_Info_GetDeviceLocalMemory, curDevice) >= 6291456)
					gos_SetScreenMode(800, 600, 16, curDevice, 0, 0, 0, true, 0, 0, 0, 0);
				else
					curDevice++;

				curIteration = 0;
			}
		}
	}
	else
	{
		gos_SetScreenMode(800, 600, 16, 0, 0, 0, 0, false, 0, true, 0, 0);
		detectionComplete = true;

		if (Environment.fullScreen == 0)
		{
			char msgBuffer[4096];
			char msgTitle[1024];

			cLoadString(IDS_SNIFFER_DONE_MSG, msgBuffer, 4095);
			cLoadString(IDS_SNIFFER_INIT_TITLE, msgTitle, 1023);

			MessageBox(NULL, msgBuffer, msgTitle, MB_OK);

			gos_TerminateApplication();
		}
	}
}

//---------------------------------------------------------------------------

void detectHardwareRender()
{
	if (gos_GetMachineInformation(gos_Info_GetDeviceLocalMemory, curDevice) >= 6291456)
	{
		DWORD startTime = timeGetTime();
		gos_SetupViewport(1, 1.0, 1, 0, 0.0, 0.0, 1.0, 1.0);		//ALWAYS FULL SCREEN for now

		if (Environment.Renderer == 3)
		{
			gos_SetRenderState(gos_State_AlphaMode, gos_Alpha_OneZero);
			gos_SetRenderState(gos_State_ShadeMode, gos_ShadeGouraud);
			gos_SetRenderState(gos_State_MonoEnable, 1);
			gos_SetRenderState(gos_State_Perspective, 0);
			gos_SetRenderState(gos_State_Clipping, 2);
			gos_SetRenderState(gos_State_AlphaTest, 0);
			gos_SetRenderState(gos_State_Specular, 0);
			gos_SetRenderState(gos_State_Dither, 0);
			gos_SetRenderState(gos_State_TextureMapBlend, gos_BlendModulate);
			gos_SetRenderState(gos_State_Filter, gos_FilterBiLinear);
			gos_SetRenderState(gos_State_TextureAddress, gos_TextureWrap);
			gos_SetRenderState(gos_State_ZCompare, 1);
			gos_SetRenderState(gos_State_ZWrite, 1);
			//DWORD fogColor = 0x009f9f9f;
			//gos_SetRenderState( gos_State_Fog, (int)&fogColor);
		}
		else
		{
			gos_SetRenderState(gos_State_AlphaMode, gos_Alpha_AlphaInvAlpha);
			gos_SetRenderState(gos_State_ShadeMode, gos_ShadeGouraud);
			gos_SetRenderState(gos_State_MonoEnable, 1);
			gos_SetRenderState(gos_State_Perspective, 1);
			gos_SetRenderState(gos_State_Clipping, 2);
			gos_SetRenderState(gos_State_AlphaTest, 1);
			gos_SetRenderState(gos_State_Specular, 1);
			gos_SetRenderState(gos_State_Dither, 1);
			gos_SetRenderState(gos_State_TextureMapBlend, gos_BlendModulateAlpha);
			gos_SetRenderState(gos_State_Filter, gos_FilterBiLinear);
			gos_SetRenderState(gos_State_TextureAddress, gos_TextureWrap);
			gos_SetRenderState(gos_State_ZCompare, 1);
			gos_SetRenderState(gos_State_ZWrite, 1);
			DWORD fogColor = 0x009f9f9f;
			gos_SetRenderState(gos_State_Fog, (int)&fogColor);
		}

		//Send down 5000 triangles
		long nIterations = 5;
		for (long i = 0; i<nIterations; i++)
		{

			DWORD totalVertices = 3000;
			gos_SetRenderState(gos_State_Texture, testTextureHandle);
			gos_RenderIndexedArray(testVertex, totalVertices, indexArray, totalVertices);
		}

		totalTime += timeGetTime() - startTime;
	}
}

//---------------------------------------------------------------------------

void detectHardwareInitialize()
{
	char msgBuffer[4096];
	char msgTitle[1024];

	cLoadString(IDS_SNIFFER_INIT_MSG, msgBuffer, 4095);
	cLoadString(IDS_SNIFFER_INIT_TITLE, msgTitle, 1023);

	MessageBox(NULL, msgBuffer, msgTitle, MB_OK);

	//-------------------------------------------------------------
	// Find the CDPath in the registry and save it off so I can
	// look in CD Install Path for files.

	//Changed for the shared source release, just set to current directory

	//DWORD maxPathLength = 1023;
	//gos_LoadDataFromRegistry("CDPath", g_cdInstallPath, &maxPathLength);
	//if (!maxPathLength)
	//	memset(g_cdInstallPath,0,1024);
	memset(g_cdInstallPath, 0, 1024);

	//Set ScreenMode to 800x600x16 FULLSCREEN!!!!
	// A window will return BAD DATA!!
	// If this fails, GAME CANNOT RUN on their machine!!!!!
	// Check which rasterizer comes up.  If software, tell prefs we cannot run in hardware!!
	while (((gos_GetMachineInformation(gos_Info_GetDeviceLocalMemory, curDevice) +
		gos_GetMachineInformation(gos_Info_GetDeviceAGPMemory, curDevice)) < 6291456) &&
		(curDevice < gos_GetMachineInformation(gos_Info_NumberDevices)))
		curDevice++;

	if (curDevice < gos_GetMachineInformation(gos_Info_NumberDevices))
		gos_SetScreenMode(800, 600, 16, curDevice, 0, 0, 0, true, 0, 0, 0, 0);

	//Create about a thousand textured random triangles.
	testVertex = (gos_VERTEX *)malloc(sizeof(gos_VERTEX) * 3000);
	indexArray = (WORD *)malloc(sizeof(WORD) * 3000);
	for (long i = 0; i<3000; i++)
	{
		testVertex[i].x = RandomNumber(1000) - 100;
		testVertex[i].y = RandomNumber(800) - 100;
		testVertex[i].z = ((float)RandomNumber(150) / 100.0f) - 0.25f;
		testVertex[i].rhw = 0.5f;
		testVertex[i].u = (float)RandomNumber(100) / 100.0f;
		testVertex[i].v = (float)RandomNumber(100) / 100.0f;
		testVertex[i].argb = 0x3fffffff;
		testVertex[i].frgb = 0x3f1f2f3f;
		indexArray[i] = i;
	}

	testTextureHandle = gos_NewTextureFromFile(gos_Texture_Solid, "testTxm.tga");
}

//---------------------------------------------------------------------------

long textToLong (char *num)
{
	long result = 0;
	
	//------------------------------------
	// Check if Hex Number
	char *hexOffset = strstr(num,"0x");
	if (hexOffset == NULL)
	{
		result = atol(num);
	}
	else
	{
		hexOffset += 2;
		long numDigits = strlen(hexOffset)-1;
		for (int i=0; i<=numDigits; i++)
		{
			if (!isalnum(hexOffset[i]) || (isalpha(hexOffset[i]) && toupper(hexOffset[i]) > 'F'))
			{
				hexOffset[i] = 0;	// we've reach a "wrong" character. Either start of a comment or something illegal. Either way, stop evaluation here.
				break;
			}
		}
		numDigits = strlen(hexOffset)-1;
		long power = 0;
		for (long count = numDigits;count >= 0;count--,power++)
		{
			unsigned char currentDigit = toupper(hexOffset[count]);
			
			if (currentDigit >= 'A' && currentDigit <= 'F')
			{
				result += (currentDigit - 'A' + 10)<<(4*power);
			}
			else if (currentDigit >= '0' && currentDigit <= '9')
			{
				result += (currentDigit - '0')<<(4*power);
			}
			else
			{
				//---------------------------------------------------------
				// There is a digit in here I don't understand.  Return 0.
				result = 0;
				break;
			}
		}
	}

	return(result);
}

//----------------------------------------------------------------------------
// Same command line Parser as MechCommander
void ParseCommandLine(char *command_line)
{
	int i;
	int n_args = 0;
	int index = 0;
	char *argv[30];
	
	char tempCommandLine[4096];
	memset(tempCommandLine,0,4096);
	strncpy(tempCommandLine,command_line,4095);

	while (tempCommandLine[index] != '\0')  // until we null out
	{
		argv[n_args] = tempCommandLine + index;
		n_args++;
		while (tempCommandLine[index] != ' ' && tempCommandLine[index] != '\0')
		{
			index++;
		}
		while (tempCommandLine[index] == ' ')
		{
			tempCommandLine[index] = '\0';
			index++;
		}
	}

	i=0;
	while (i<n_args)
	{
		if (strcmpi(argv[i],"-mission") == 0)
		{
			i++;
			if (i < n_args)
			{
				g_dbgLoadMission = true;
				if (argv[i][0] == '"')
				{
					// They typed in a quote, keep reading argvs
					// until you find the close quote
					strcpy(g_missionName,&(argv[i][1]));
					bool scanName = true;
					while (scanName && (i < n_args))
					{
						i++;
						if (i < n_args)
						{
							strcat(g_missionName," ");
							strcat(g_missionName,argv[i]);

							if (strstr(argv[i],"\"") != NULL)
							{
								scanName = false;
								g_missionName[strlen(g_missionName)-1] = 0;
							}
						}
						else
						{
							//They put a quote on the line with no space.
							scanName = false;
							g_missionName[strlen(g_missionName)-1] = 0;
						}
					}
				}
				else
					strcpy(g_missionName,argv[i]);
			}
		}
		else if (strcmpi(argv[i],"-sniffer") == 0)
		{
			detectHardwareMode = true;
		}
		else if (strcmpi(argv[i], "-braindead") == 0) {
			i++;
			if (i < n_args) {
				long teamID = textToLong(argv[i]);
				MechWarrior::brainsEnabled[teamID] = false;
			}
		}
		else if (strcmpi(argv[i], "-turrets_off") == 0) { // MCHD TODO: Make this a runtime debug cheat
			i++;
			if (i < n_args) {
				long teamID = textToLong(argv[i]);
				(void)teamID;
				//Turret::turretsEnabled[teamID] = false;
			}
		}
		else if (strcmpi(argv[i], "-debugwins") == 0) {
			i++;
			if (i < n_args) {
				long winState = textToLong(argv[i]);
				if (winState == 1)
					DebugWindowOpen[0] = true;
				else if (winState == 2) {
					DebugWindowOpen[1] = true;
					DebugWindowOpen[2] = true;
					DebugWindowOpen[3] = true;
					}
				else if (winState == 3) {
					DebugWindowOpen[0] = true;
					DebugWindowOpen[1] = true;
					DebugWindowOpen[2] = true;
					DebugWindowOpen[3] = true;
				}
			}
		}
		else if (strcmpi(argv[i], "-objectwins") == 0) {
			i++;
			if (i < n_args) {
				long partNumber = textToLong(argv[i]);
				if (NumGameObjectsToDisplay < 3)
					GameObjectWindowList[NumGameObjectsToDisplay++] = partNumber;
			}
		}
		else if (strcmpi(argv[i], "-debugcells") == 0) {
			i++;
			if (i < n_args) {
				long setting = textToLong(argv[i]);
				if ((setting > 0) && (setting < 5))
					g_dbgDrawCombatMove = setting;
			}
		}
		else if (strcmpi(argv[i], "-nopain") == 0) { // MCHD TODO: Make this a runtime debug cheat
			i++;
			if (i < n_args) {
				long teamID = textToLong(argv[i]);
				(void)teamID;
				//Team::noPain[teamID] = true;
			}
		}
		else if (strcmpi(argv[i], "-disable") == 0) {
			i++;
			if (i < n_args) {
				long partID = textToLong(argv[i]);
				g_dbgDisableAtStartIds[g_dbgNumDisableAtStart++] = partID;
			}
		}
		else if (strcmpi(argv[i], "-log") == 0) {
			i++;
			initGameLogs = true;
			if (i < n_args) {
				if (strcmpi(argv[i], "net") == 0)
					initNetLog = true;
				if (strcmpi(argv[i], "weaponfire") == 0)
					initCombatLog = true;
				if (strcmpi(argv[i], "bugs") == 0)
					initBugLog = true;
				if (strcmpi(argv[i], "lrmove") == 0)
					initLRMoveLog = true;
			}
		}
		else if (strcmpi(argv[i], "-show") == 0) {
			i++;
			if (i < n_args) {
				if (strcmpi(argv[i], "movers") == 0)
					g_dbgShowMovers = true;
			}
		}
		else if (strcmpi(argv[i], "-killambient") == 0) {
			g_disableAmbientLight = true;
		}
		else if (strcmpi(argv[i], "-rps") == 0) {
			i++;
			if (i < n_args)
				g_dbgResourcePoints = textToLong(argv[i]);
		}
		else if (strcmpi(argv[i], "-registerzone") == 0) {
			MultiPlayer::registerZone = true;
		}
		else if (strcmpi(argv[i], "-dropzones") == 0) {
			i++;
			if (i < n_args) {
				long numPlayers = strlen(argv[i]);
				for (long j = 0; j < numPlayers; j++)
					MultiPlayer::presetDropZones[j] = (argv[i][j] - '0');
			}
		}

		i++;
	}
}

//---------------------------------------------------------------------------
// Setup the GameOS structure -- This tells GameOS what I am using
void GetGameOSEnvironment( char* CommandLine )
{
	ParseCommandLine(CommandLine);

	Environment.applicationName			= "MechCommander2\\1.0"; // MP012001";

	Environment.debugLog				= "";	// MCHD TODO: Test this out
	Environment.spew					= "";
	Environment.TimeStampSpew			= 0;

	Environment.GetGameInformation		= GetGameInformation;
	Environment.UpdateRenderers			= Render;
	Environment.InitializeGameEngine	= Initialize;
	Environment.DoGameLogic				= UpdateGame;
	Environment.TerminateGameEngine		= Terminate;

	Environment.Renderer				= 0;

	Environment.allowMultipleApps = false;
	Environment.dontClearRegistry = true;
	
	Environment.soundDisable			= false;
	Environment.soundHiFi				= true;
	Environment.soundChannels			= 20;

	//--------------------------
	// Networking information...
	// MechCommander GUID = {09608800-4815-11d2-92D2-0060973CFB2C}
	// or {0x9608800, 0x4815, 0x11d2, {0x92, 0xd2, 0x0, 0x60, 0x97, 0x3c, 0xfb, 0x2c}}
	Environment.NetworkGame = false;
	Environment.NetworkMaxPlayers = MAX_COMMANDERS;
	Environment.NetworkGUID[0] = 0x09;
	Environment.NetworkGUID[1] = 0x60;
	Environment.NetworkGUID[2] = 0x88;
	Environment.NetworkGUID[3] = 0x00;
	Environment.NetworkGUID[4] = 0x48;
	Environment.NetworkGUID[5] = 0x15;
	Environment.NetworkGUID[6] = 0x11;
	Environment.NetworkGUID[7] = 0xd2;
	Environment.NetworkGUID[8] = 0x92;
	Environment.NetworkGUID[9] = 0xd2;
	Environment.NetworkGUID[10] = 0x00;
	Environment.NetworkGUID[11] = 0x60;
	Environment.NetworkGUID[12] = 0x97;
	Environment.NetworkGUID[13] = 0x3c;
	Environment.NetworkGUID[14] = 0xfb;
	Environment.NetworkGUID[15] = 0x2c;
	
	Environment.Key_FullScreen			= 0;
	Environment.Key_SwitchMonitors		= 0;
	Environment.Key_Exit				= 0;
	Environment.screenWidth				= 800;
	Environment.screenHeight			= 600;
	Environment.bitDepth				= 16;
	Environment.fullScreen				= 0;

	HKEY hKey;
	LONG result;
	char pData[1024];
	DWORD szData = 1023;

	result=RegOpenKey(HKEY_CURRENT_USER, GAME_REG_KEY, &hKey);
	if( ERROR_SUCCESS==result )
	{
		result=RegQueryValueEx(hKey,"FIRSTRUN",NULL,NULL,(PBYTE)pData,&szData);
		if ((result == ERROR_SUCCESS) && pData[0])
		{
			if (fileExists("options.cfg") && !detectHardwareMode)
#ifndef LAB_ONLY
				Environment.fullScreen				= 1;
#endif
			g_EULAShown = true;
		}

		RegCloseKey(hKey);
	}

	Environment.version					= versionStamp;
	Environment.FullScreenDevice		= 0;
	Environment.AntiAlias				= 0;
	Environment.disableZBuffer			= 0;

// Texture infomation
	Environment.Texture_S_256			= 6;
	Environment.Texture_S_128			= 1;
	Environment.Texture_S_64			= 0;
	Environment.Texture_S_32			= 1;
	Environment.Texture_S_16			= 5;

	Environment.Texture_K_256			= 2;
	Environment.Texture_K_128			= 5;
	Environment.Texture_K_64			= 5;
	Environment.Texture_K_32			= 5;
	Environment.Texture_K_16			= 5;

	Environment.Texture_A_256			= 0;
	Environment.Texture_A_128			= 1;
	Environment.Texture_A_64			= 5;
	Environment.Texture_A_32			= 1;
	Environment.Texture_A_16			= 0;

	Environment.RaidDataSource			= "MechCommander 2:Raid4"; 
	Environment.RaidFilePath			= "\\\\aas1\\MC2\\Test\\GOSRaid";
	Environment.RaidCustomFields		= "Area=GOSRaid"; 	

	Environment.DisableLowEndCard		= 1;
	Environment.Suppress3DFullScreenWarning = 1;
	Environment.RenderToVram			= 1;
}

//***************************************************************************
// DEBUGGING WINDOWS
//***************************************************************************

void DEBUGWINS_init(void) {

	sprintf(DebugStatusBarString, "DEBUG Status Bar: GLENNBA");

	DebugWindow[0] = new GameDebugWindow;
	DebugWindow[0]->init();
	DebugWindow[0]->setPos(10, 200);
	if (DebugWindowOpen[0])
		DebugWindow[0]->open();
	else
		DebugWindow[0]->close();

	DebugWindow[1] = new GameDebugWindow;
	DebugWindow[1]->init();
	DebugWindow[1]->setPos(10, 20);
	if (DebugWindowOpen[1])
		DebugWindow[1]->open();
	else
		DebugWindow[1]->close();

	DebugWindow[2] = new GameDebugWindow;
	DebugWindow[2]->init();
	DebugWindow[2]->setPos(220, 20);
	if (DebugWindowOpen[2])
		DebugWindow[2]->open();
	else
		DebugWindow[2]->close();

	DebugWindow[3] = new GameDebugWindow;
	DebugWindow[3]->init();
	DebugWindow[3]->setPos(430, 20);
	if (DebugWindowOpen[3])
		DebugWindow[3]->open();
	else
		DebugWindow[3]->close();
}

//---------------------------------------------------------------------------

void DEBUGWINS_destroy(void) {

	for (long i = 0; i < NUM_DEBUG_WINDOWS; i++)
		if (DebugWindow[i]) {
			DebugWindow[i]->destroy();
			delete DebugWindow[i];
			DebugWindow[i] = NULL;
		}
}

//---------------------------------------------------------------------------

void initDialogs()
{
	FullPathFileName path;
	path.init(artPath, "mcl_dialog", ".fit");

	FitIniFile file;
	if (NO_ERR != file.open(path))
	{
		char error[256];
		sprintf(error, "couldn't open file %s", (char*)path);
		Assert(0, 0, error);
		return;
	}

	LogisticsOKDialog::init(file);
	file.close();

	path.init(artPath, "mcl_sm", ".fit");

	if (NO_ERR != file.open(path))
	{
		char error[256];
		sprintf(error, "couldn't open file %s", (char*)path);
		Assert(0, 0, error);
		return;
	}

	LogisticsSaveDialog::init(file);
	file.close();

	path.init(artPath, "mcl_dialog_onebutton", ".fit");
	if (NO_ERR != file.open(path))
	{
		char error[256];
		sprintf(error, "couldn't open file %s", (char*)path);
		Assert(0, 0, error);
		return;
	}

	LogisticsOneButtonDialog::init(file);
}

//---------------------------------------------------------------------------

void endDialogs()
{
	if (LogisticsOKDialog::s_instance)
	{
		delete LogisticsOKDialog::s_instance;
		LogisticsOKDialog::s_instance = NULL;
	}

	if (LogisticsSaveDialog::s_instance)
	{
		delete LogisticsSaveDialog::s_instance;
		LogisticsSaveDialog::s_instance = NULL;
	}

	if (LogisticsOneButtonDialog::s_instance)
	{
		delete LogisticsOneButtonDialog::s_instance;
		LogisticsOneButtonDialog::s_instance = NULL;
	}

	if (LogisticsLegalDialog::s_instance)
	{
		delete LogisticsLegalDialog::s_instance;
		LogisticsLegalDialog::s_instance = NULL;
	}
}

//---------------------------------------------------------------------------

void DEBUGWINS_toggle(bool* windowsOpen) {

	for (long i = 0; i < NUM_DEBUG_WINDOWS; i++)
		if (windowsOpen[i])
			DebugWindow[i]->toggle();
	DebugHelpOpen = windowsOpen[NUM_DEBUG_WINDOWS];
	DebugStatusBarOpen = windowsOpen[NUM_DEBUG_WINDOWS + 1];
	DebugScoreBoardOpen = windowsOpen[NUM_DEBUG_WINDOWS + 2];
}

//---------------------------------------------------------------------------

void DEBUGWINS_display(bool* windowsOpen) {

	for (long i = 0; i < NUM_DEBUG_WINDOWS; i++) {
		if (windowsOpen[i]) {
			DebugWindowOpen[i] = true;
			DebugWindow[i]->open();
		}
		else {
			DebugWindowOpen[i] = false;
			DebugWindow[i]->close();
		}
	}
	DebugHelpOpen = windowsOpen[NUM_DEBUG_WINDOWS];
	DebugStatusBarOpen = windowsOpen[NUM_DEBUG_WINDOWS + 1];
	DebugScoreBoardOpen = windowsOpen[NUM_DEBUG_WINDOWS + 2];
}

//---------------------------------------------------------------------------

void DEBUGWINS_print(char* s, long window) {

	DebugWindow[window]->print(s);
}

//---------------------------------------------------------------------------

void DEBUGWINS_setGameObject(long debugObj, GameObjectPtr obj) {

	if (debugObj == -1) {
		DebugGameObject[2] = DebugGameObject[1];
		DebugGameObject[1] = DebugGameObject[0];
		debugObj = 0;
	}
	DebugGameObject[debugObj] = obj;
}

//---------------------------------------------------------------------------

void DEBUGWINS_viewGameObject(long debugObj) {

	if (DebugGameObject[debugObj]) {
		Stuff::Vector3D newPos = DebugGameObject[debugObj]->getPosition();
		eye->setPosition(newPos, false);
	}
}

//---------------------------------------------------------------------------

void DEBUGWINS_removeGameObject(GameObjectPtr obj) {

	for (long i = 0; i < 3; i++)
		if (DebugGameObject[i] == obj) {
			DebugGameObject[i] = NULL;
			DebugWindow[1 + i]->clear();
		}
}

//---------------------------------------------------------------------------

void DEBUGWINS_update(void) {

	for (long i = 0; i < 3; i++)
		if (DebugGameObject[i])
			DebugGameObject[i]->updateDebugWindow(DebugWindow[1 + i]);
}

//---------------------------------------------------------------------------

void DEBUGWINS_renderSpecialWindows(void) {

	gos_TextSetAttributes(DebugWindow[0]->font, 0xffffffff, 1.0, true, true, false, false);
	gos_TextSetRegion(0, 0, Environment.screenWidth, Environment.screenHeight);
	gos_TextSetPosition(15, 10);
	if (DebugStatusBarOpen && DebugStatusBarString[0])
		gos_TextDraw(DebugStatusBarString);
	if (DebugScoreBoardOpen) {
		if (MPlayer) {
			long curY = Environment.screenHeight - 390;
			for (long i = 0; i < MPlayer->numTeams; i++) {
				char s[256];
				sprintf(s, "Team %ld score = %ld", i, MPlayer->teamScore[i]);
				gos_TextSetPosition(Environment.screenWidth - 380, curY);
				gos_TextDraw(s);
				curY += 10;
			}
			curY += 10;
			for (i = 0; i < MAX_MC_PLAYERS; i++)
				if (MPlayer->playerInfo[i].commanderID > -1) {
					char s[256];
					sprintf(s, "Player %d (%s) score = %ld, %ld kills, %ld losses",
						i,
						MPlayer->playerInfo[i].name,
						MPlayer->playerInfo[i].score,
						MPlayer->playerInfo[i].kills,
						MPlayer->playerInfo[i].losses);
					gos_TextSetPosition(Environment.screenWidth - 380, curY);
					gos_TextDraw(s);
					curY += 10;
				}
		}
		else {
			gos_TextSetPosition(Environment.screenWidth - 275, Environment.screenHeight - 120);
			gos_TextDraw("Single-player Mission");
		}
	}
	if (DebugHelpOpen) {
		gos_TextSetPosition(15, 10);
		gos_TextDraw("DEBUGGING COMMANDS");
		gos_TextSetPosition(15, 20);
		gos_TextDraw("----------------------------------------------------------------------------");

		gos_TextSetPosition(15, 30);
		gos_TextDraw("ALT-W");
		gos_TextSetPosition(140, 30);
		gos_TextDraw("Toggle Debug Windows");

		gos_TextSetPosition(15, 40);
		gos_TextDraw("ALT-T");
		gos_TextSetPosition(140, 40);
		gos_TextDraw("Teleport to mouse");

		gos_TextSetPosition(15, 50);
		gos_TextDraw("ALT-/");
		gos_TextSetPosition(140, 50);
		gos_TextDraw("Quick Info");

		gos_TextSetPosition(15, 60);
		gos_TextDraw("ALT-SHFT-/");
		gos_TextSetPosition(140, 60);
		gos_TextDraw("Select Debug Object");

		gos_TextSetPosition(15, 70);
		gos_TextDraw("ALT-#");
		gos_TextSetPosition(140, 70);
		gos_TextDraw("Center on Debug Object # (1 thru 3)");

		gos_TextSetPosition(15, 80);
		gos_TextDraw("ALT-CTRL-#");
		gos_TextSetPosition(140, 80);
		gos_TextDraw("Toggle Info for Debug Object (1 thru 3)");

		gos_TextSetPosition(15, 90);
		gos_TextDraw("ALT-M");
		gos_TextSetPosition(140, 90);
		gos_TextDraw("Show All Movers (toggle)");

		gos_TextSetPosition(15, 100);
		gos_TextDraw("ALT-G");
		gos_TextSetPosition(140, 100);
		gos_TextDraw("GlobalMap Move Log (toggle)");

		gos_TextSetPosition(15, 110);
		gos_TextDraw("ALT-B");
		gos_TextSetPosition(140, 110);
		gos_TextDraw("Enemies Brain Dead (toggle)");

		gos_TextSetPosition(15, 120);
		gos_TextDraw("ALT-P");
		gos_TextSetPosition(140, 120);
		gos_TextDraw("Use Goal Planning (toggle on selected movers)");

		gos_TextSetPosition(15, 130);
		gos_TextDraw("ALT-CTRL-P");
		gos_TextSetPosition(140, 130);
		gos_TextDraw("Computer AI Use Goal Planning (toggle)");

		gos_TextSetPosition(15, 140);
		gos_TextDraw("ALT-4");
		gos_TextSetPosition(140, 140);
		gos_TextDraw("Damage Object for 1 point");

		gos_TextSetPosition(15, 150);
		gos_TextDraw("ALT-5");
		gos_TextSetPosition(140, 150);
		gos_TextDraw("Damage Object for 4 points");

		gos_TextSetPosition(15, 160);
		gos_TextDraw("ALT-6");
		gos_TextSetPosition(140, 160);
		gos_TextDraw("Damage Object for 9 points");

		gos_TextSetPosition(15, 170);
		gos_TextDraw("ALT-7");
		gos_TextSetPosition(140, 170);
		gos_TextDraw("Damage Object for 16 points");

		gos_TextSetPosition(15, 180);
		gos_TextDraw("ALT-8");
		gos_TextSetPosition(140, 180);
		gos_TextDraw("Damage Object for 25 points");

		gos_TextSetPosition(15, 190);
		gos_TextDraw("ALT-9");
		gos_TextSetPosition(140, 190);
		gos_TextDraw("Damage Object for 36 points");

		gos_TextSetPosition(15, 200);
		gos_TextDraw("ALT-0");
		gos_TextSetPosition(140, 200);
		gos_TextDraw("Disable Object");

		gos_TextSetPosition(15, 210);
		gos_TextDraw("ALT-V");
		gos_TextSetPosition(140, 210);
		gos_TextDraw("Display TacOrder Target in Debug Window");
	}
}

//---------------------------------------------------------------------------

void DEBUGWINS_render(void) {

	for (long i = 0; i < NUM_DEBUG_WINDOWS; i++)
		DebugWindow[i]->render();
	DEBUGWINS_renderSpecialWindows();
}

//***************************************************************************