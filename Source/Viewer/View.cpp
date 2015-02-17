//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
// Viewer.cpp : Defines the class behaviors for the application.
//


#ifndef VERSION_H
#include "version.h"
#endif

#include <GameOS.hpp>
#include <ToolOS.hpp>

#include "..\resource.h"
#include "mclib.h"
#include "GameSound.h"
#include <windows.h>
#include "mission.h"
#include "Mechlopedia.h"
#include "LogisticsData.h"
#include "prefs.h"
#include "Soundsys.h"

CPrefs g_userPreferences;
SoundSystem*	 g_soundSystem = NULL;
MultiPlayer* MPlayer = NULL;

extern float frameRate;

// globals used for memory
UserHeapPtr g_systemHeap = NULL;
UserHeapPtr g_GUIHeap = NULL;

//unsigned long elementHeapSize = 1024000;
//unsigned long maxElements = 2048;
//unsigned long maxGroups = 1024;
#define KILOBYTE_TO_BYTES 1024
#define MEGABYTES_TO_BYTES (KILOBYTE_TO_BYTES * KILOBYTE_TO_BYTES)
unsigned long g_systemHeapSize = 256 * MEGABYTES_TO_BYTES;
unsigned long g_guiHeapSize = 4 * MEGABYTES_TO_BYTES;
unsigned long g_terrainHeapSize = 16 * MEGABYTES_TO_BYTES;
unsigned long g_spriteHeapSize = 64 * MEGABYTES_TO_BYTES;
unsigned long g_maxVertexCount = 1000000;
unsigned long tglHeapSize = 256 * MEGABYTES_TO_BYTES;

bool		  GeneralAlarm = 0;

extern char g_FileMissingString[];
extern char g_CDMissingString[];
extern char g_MissingTitleString[];

extern char g_cdInstallPath[];

// long FilterState = gos_FilterNone;
long gammaLevel = 0;
long g_renderer = 0;
long g_gameDifficulty = 0;
bool g_unlimitedAmmo = true;

Camera* eye = NULL;
enum { CPU_UNKNOWN, CPU_PENTIUM, CPU_MMX, CPU_KATMAI } Processor = CPU_PENTIUM;		//Needs to be set when GameOS supports ProcessorID -- MECHCMDR2

long	g_objectTextureSize = 128;
char	g_missionName[1024];
float	gosFontScale = 1.0;

float   doubleClickThreshold = 0.2f;
float	dragThreshold = 0.016667f;


DWORD gosResourceHandle = 0;
HGOSFONT3D gosFontHandle = 0;


bool g_quitGame = FALSE;


// these globals are necessary for fast files for some reason
FastFile 	**g_fastFiles = NULL;
long 		g_numFastFiles = 0;
long		g_maxFastFiles = 0;

char*	ExceptionGameMsg = "";

bool	useLOSAngle = 0;

Stuff::MemoryStream *effectStream = NULL;
extern MidLevelRenderer::MLRClipper * theClipper;



Mechlopedia*	pMechlopedia;
LogisticsData*  pLogData;


char *SpecialtySkillsTable[NUM_SPECIALTY_SKILLS] = {
	"LightMechSpecialist",
	"LaserSpecialist",
	"LightACSpecialist",
	"MediumACSpecialist",
	"SRMSpecialist",
	"SmallArmsSpecialist",
	"SensorProfileSpecialist",
	"ToughnessSpecialist",			//Thoughness Specialty

	"MediumMechSpecialist",
	"PulseLaserSpecialist",
	"ERLaserSpecialist",
	"LRMSpecialist",
	"Scout",						//Scouting Specialty
	"LongJump",						//Jump Jet Specialty

	"HevayMechSpecialist",			//Heavy mech Specialty
	"PPCSpecialist",
	"HeavyACSpecialist",
	"ShortRangeSpecialist",
	"MediumRangeSpecialist",
	"LongRangeSpecialist",

	"AssaultMechSpecialist",
	"GaussSpecialist",
	"SharpShooter",					//Sharpshooter specialty
};

// called by gos
//---------------------------------------------------------------------------
char* __stdcall GetGameInformation() 
{
	return(ExceptionGameMsg);
}

// called by GOS when you need to draw
//---------------------------------------------------------------------------
void __stdcall UpdateRenderers()
{
	
	DWORD bColor = 0x0;
	
 	gos_SetupViewport(1,1.0,1,bColor, 0.0, 0.0, 1.0, 1.0 );		//ALWAYS FULL SCREEN for now

	gos_SetRenderState( gos_State_Filter, gos_FilterBiLinear );
	gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_AlphaInvAlpha );

	gos_SetRenderState( gos_State_AlphaTest, TRUE );

	gos_SetRenderState( gos_State_Clipping, TRUE);

	gos_SetRenderState( gos_State_TextureAddress, gos_TextureClamp );

	gos_SetRenderState( gos_State_Dither, 1);
	
	float viewMulX, viewMulY, viewAddX, viewAddY;

	gos_GetViewport(&viewMulX, &viewMulY, &viewAddX, &viewAddY);
			
	//------------------------------------------------------------
	gos_SetRenderState( gos_State_Filter, gos_FilterNone );

	pMechlopedia->render();
	
	gos_SetRenderState( gos_State_Filter, gos_FilterNone );
	userInput->setViewport(viewMulX,viewMulY,viewAddX,viewAddY);
	userInput->render();

}

//------------------------------------------------------------
void __stdcall DoGameLogic()
{
	//---------------------------------------------------------------
	// Somewhere in all of the updates, we have asked to be excused!
	if (g_quitGame)
	{
		//EnterWindowMode();				//Game crashes if _TerminateApp called from fullScreen
		gos_TerminateApplication();
	}

	if (frameRate < Stuff::SMALL)
		frameRate = 4.0f;

	g_frameTime = 1.0 / frameRate;
	if (g_frameTime > 0.25f)
		g_frameTime = 0.25f;

	userInput->update();

	g_gameSoundSystem->update();

	pMechlopedia->update();

	if ( LogisticsScreen::RUNNING != pMechlopedia->getStatus() )
		g_quitGame = true;
}



//---------------------------------------------------------------------------
void __stdcall InitializeGameEngine()
{
 	gosResourceHandle = gos_OpenResourceDLL("mc2res.dll");
	
	char temp[256];
	cLoadString( IDS_FLOAT_HELP_FONT, temp, 255 );
	char* pStr = strstr( temp, "," );
	if ( pStr )
	{
		gosFontScale = atoi( pStr + 2 );
		*pStr = 0;
	}
	char path [256];
	strcpy( path, "assets\\graphics\\" );
	strcat( path, temp );	

	gosFontHandle = gos_LoadFont(path);

   	//-------------------------------------------------------------
   	// Find the CDPath in the registry and save it off so I can
   	// look in CD Install Path for files.
	//Changed for the shared source release, just set to current directory
	//DWORD maxPathLength = 1023;
	//gos_LoadDataFromRegistry("CDPath", g_cdInstallPath, &maxPathLength);
	//if (!maxPathLength)
	//	strcpy(g_cdInstallPath,"..\\");
	strcpy(g_cdInstallPath,".\\");

	cLoadString(IDS_MC2_FILEMISSING,g_FileMissingString,511);
	cLoadString(IDS_MC2_CDMISSING,g_CDMissingString,1023);
	cLoadString(IDS_MC2_MISSING_TITLE,g_MissingTitleString,255);

	//--------------------------------------------------------------
	// Start the SystemHeap and globalHeapList
	globalHeapList = new HeapList;
	gosASSERT(globalHeapList != NULL);

	globalHeapList->init();
	globalHeapList->update();		//Run Instrumentation into GOS Debugger Screen

	g_systemHeap = new UserHeap;
	gosASSERT(g_systemHeap != NULL);

	g_systemHeap->init(g_systemHeapSize,"SYSTEM");
	
	float doubleClickThreshold = 0.2f;

	//--------------------------------------------------------------
	// Read in System.CFG
	FitIniFile systemFile;

#ifdef _DEBUG
	long systemOpenResult = 
#endif
		systemFile.open("system.cfg");
		   
#ifdef _DEBUG
	if( systemOpenResult != NO_ERR)
	{
		char Buffer[256];
		gos_GetCurrentPath( Buffer, 256 );
		STOP(( "Cannot find \"system.cfg\" file in %s",Buffer ));
	}
#endif

	{
#ifdef _DEBUG
		long systemBlockResult = 
#endif
		systemFile.seekBlock("g_systemHeap");
		gosASSERT(systemBlockResult == NO_ERR);
		{
			long result = systemFile.readIdULong("systemHeapSize",g_systemHeapSize);
			gosASSERT(result == NO_ERR);
		}

#ifdef _DEBUG
		long systemPathResult = 
#endif
		systemFile.seekBlock("systemPaths");
		gosASSERT(systemPathResult == NO_ERR);
		{
			long result = systemFile.readIdString("terrainPath",terrainPath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("artPath",artPath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("fontPath",fontPath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("savePath",savePath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("spritePath",spritePath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("shapesPath",shapesPath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("soundPath",soundPath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("objectPath",objectPath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("cameraPath",cameraPath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("tilePath",tilePath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("missionPath",missionPath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("warriorPath",warriorPath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("profilePath",profilePath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("interfacepath",interfacePath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("moviepath",moviePath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("CDsoundPath",CDsoundPath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("CDmoviepath",CDmoviePath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("tglPath",tglPath,79);
			gosASSERT(result == NO_ERR);

			result = systemFile.readIdString("texturePath",texturePath,79);
			gosASSERT(result == NO_ERR);
		}

#ifdef _DEBUG
		long fastFileResult = 
#endif
		systemFile.seekBlock("FastFiles");
		gosASSERT(fastFileResult == NO_ERR);
		{
			long result = systemFile.readIdLong("NumFastFiles",g_maxFastFiles);
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
	
				while (systemFile.readIdString(fastFileId,fileName,99) == NO_ERR)
				{
					bool result = FastFileInit(fileName);
					if (!result)
						STOP(("Unable to startup fastfiles.  Probably an old one in the directory!!"));

					fileNum++;
					sprintf(fastFileId,"File%ld",fileNum);
				}
			}
		}
	}

	systemFile.close();

	//--------------------------------------------------------------
	// MCHD CHANGE (02/14/2015): Preferences loading block deleted - everything moved to g_userPreferences.load()

	// Load and apply options from "options.cfg"
	g_userPreferences.load();
	g_userPreferences.applyPrefs();

 	//-------------------------------
	// Used to output debug stuff!
	// Mondo COOL!
	// simply do this in the code and stuff goes to the file called mc2.output
	//		DEBUG_STREAM << thing_you_want_to_output
	//
	// IMPORTANT NOTE:
	Stuff::InitializeClasses();
	MidLevelRenderer::InitializeClasses(8192*4,1024,0,0,true);
	gosFX::InitializeClasses();
	
	gos_PushCurrentHeap(MidLevelRenderer::Heap);

	MidLevelRenderer::TGAFilePool *pool = new MidLevelRenderer::TGAFilePool("data\\Effects\\");
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
	effectsName.init(effectsPath,"mc2.fx","");

	File effectFile;
	long result = effectFile.open(effectsName);
	if (result != NO_ERR)
		STOP(("Could not find MC2.fx"));
		
	long effectsSize = effectFile.fileSize();
	MemoryPtr effectsData = (MemoryPtr)g_systemHeap->Malloc(effectsSize);
	effectFile.read(effectsData,effectsSize);
	effectFile.close();
	
	effectStream = new Stuff::MemoryStream(effectsData,effectsSize);
	gosFX::EffectLibrary::Instance->Load(effectStream);
	
	gosFX::LightManager::Instance = new gosFX::LightManager();

	gos_PopCurrentHeap();

	g_systemHeap->Free(effectsData);
	

	//------------------------------------------------
	// Fire up the MC Texture Manager.
	mcTextureManager = new MC_TextureManager;
	mcTextureManager->start();

	//Startup the vertex array pool
	mcTextureManager->startVertices(g_maxVertexCount);

	//--------------------------------------------------
	// Setup Mouse Parameters from Prefs.CFG
	userInput = new UserInput;
	userInput->init();
	userInput->setMouseDoubleClickThreshold(doubleClickThreshold);
	userInput->setMouseDragThreshold(dragThreshold);
	userInput->initMouseCursors( "cursors" );
	userInput->setMouseCursor( mState_NORMAL );
	userInput->mouseOn();



	// now the sound system
	g_gameSoundSystem = new GameSoundSystem;
	g_gameSoundSystem->init();
	((SoundSystem *)g_gameSoundSystem)->init("sound");
	g_soundSystem = g_gameSoundSystem; // for things in the lib that use sound
	g_gameSoundSystem->playDigitalMusic( LOGISTICS_LOOP );

	
	pLogData = new LogisticsData;
	pLogData->init();
	

	pMechlopedia = new Mechlopedia;
	pMechlopedia->init();
	pMechlopedia->begin();




}

void __stdcall TerminateGameEngine()
{

	if ( pMechlopedia )
		delete pMechlopedia;

	if ( userInput )
		delete userInput;

	if ( g_gameSoundSystem )
		delete g_gameSoundSystem;

	if ( pLogData )
		delete pLogData;

	//------------------------------------------------
	// shutdown the MC Texture Manager.
	if (mcTextureManager)
	{
		mcTextureManager->destroy();

		delete mcTextureManager;
		mcTextureManager = NULL;
	}

	//--------------------------------------------------------------
	// End the SystemHeap and globalHeapList
	if (g_systemHeap)
	{
		g_systemHeap->destroy();

		delete g_systemHeap;
		g_systemHeap = NULL;
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

	delete effectStream;
	delete gosFX::LightManager::Instance;

	gos_PopCurrentHeap();

	//
	//-------------------
	// Turn off libraries
	//-------------------
	//
	gosFX::TerminateClasses();
	MidLevelRenderer::TerminateClasses();
	Stuff::TerminateClasses();

	//Redundant.  Something else is shutting this down.
	//GOS sure does think its bad to delete something multiple times though.
	//Even though it simply never is!
	//gos_DeleteFont(gosFontHandle);

	gos_CloseResourceDLL(gosResourceHandle);

	//
	//--------------------------
	// Turn off the fast Files
	//--------------------------
	//
	FastFileFini();
	
	
}


//---------------------------------------------------------------------
void __stdcall GetGameOSEnvironment( char* CommandLine )
{
	Environment.applicationName			= "MechCommander 2 Encyclopedia";

	Environment.debugLog				= "";			//"DebugLog.txt";
	Environment.memoryTraceLevel		= 5;
	Environment.spew					= ""; //"GameOS_Texture GameOS_DirectDraw GameOS_Direct3D ";
	Environment.TimeStampSpew			= 0;

	Environment.GetGameInformation		= GetGameInformation;
	Environment.UpdateRenderers			= UpdateRenderers;
	Environment.InitializeGameEngine	= InitializeGameEngine;
	Environment.DoGameLogic				= DoGameLogic;
	Environment.TerminateGameEngine		= TerminateGameEngine;
	
	Environment.soundDisable			= false;
	Environment.soundHiFi				= true;
	Environment.soundChannels			= 24;

	Environment.version					= versionStamp;

	Environment.AntiAlias				= 0;
//
// Texture infomation
//
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

	Environment.bitDepth				= 16;

	Environment.RaidDataSource			= "MechCommander 2:Raid4"; 
	Environment.RaidFilePath			= "\\\\aas1\\MC2\\Test\\GOSRaid";
	Environment.RaidCustomFields		= "Area=GOSRaid"; 	

	
	Environment.screenWidth = 800;
	Environment.screenHeight = 600;
}


