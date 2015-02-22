//--------------------------------------------------------------------------------------
// MechCommander 2
// This header contains the mission classes for the GUI
// GUI is now single update driven.  An event comes in, the manager decides who its for
// and passes the event down.  Eveything still know how to draw etc.
// All drawing is done through gos_drawQuad and drawTriangle
// Basic cycle is 
//		call GObject->update with this frame's events.
//			This will check the events to see if any pertain to me.
//			Draw anything to the texture plane that needs drawing.
//			Call any code which the events might have trigged.
//		call GObject->render with this frame's events.
//			This draws the object to the screen.
//			called in order of depth.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//--------------------------------------------------------------------------------------
// Include Files
#include "missiongui.h"
#include "objmgr.h"
#include "gamesound.h"
#include "sounds.h"
#include "multplyr.h"
#include "team.h"
#include "gamecam.h"
#include "Vertex.h"
#include "Mech.h"
#include "artlry.h"
#include "LogisticsPilot.h"
#include "LogisticsData.h"
#include "Mission.h"
#include "group.h"
#include "movemgr.h"
#include "comndr.h"
#include "Prefs.h"
#include "KeyboardRef.h"
#include "LogisticsDialog.h"
#include "..\resource.h"
#include "windows.h"
#include "gvehicl.h" // remove

	static char* terrainStr[NUM_TERRAIN_TYPES] = {
			"Blue Water",	//MC_BLUEWATER_TYPE
			"Green Water",	//MC_GREEN_WATER_TYPE
			"Mud",	//MC_MUD_TYPE
			"Moss",	//MC_MOSS_TYPE
			"Dirt",	//MC_DIRT_TYPE
			"Ash",	//MC_ASH_TYPE
			"Mountain",	//MC_MOUNTAIN_TYPE
			"Tundra",	//MC_TUNDRA_TYPE
			"Forest",	//MC_FORESTFLOOR_TYPE
			"Grass",	//MC_GRASS_TYPE
			"Concrete",	//MC_CONCRETE_TYPE
			"Cliff",	//MC_CLIFF_TYPE
			"Slimy",	//MC_SLIMY_TYPE
			"None"	//MC_NONE_TYPE
		};

extern void testKeyStuff();

extern long helpTextHeaderID;
extern long helpTextID;
extern CPrefs g_userPreferences;

extern bool invulnerableON;
extern bool MLRVertexLimitReached;

//--------------------------------------------------------------------------------------
// Globals
extern float g_frameTime;

extern void DEBUGWINS_print (char* s, long window = 0);
extern void DEBUGWINS_setGameObject (long debugObj, GameObjectPtr obj);
extern void DEBUGWINS_toggle (bool* windowsOpen);
extern void DEBUGWINS_display (bool* windowsOpen);
extern void DEBUGWINS_viewGameObject (long debugObj);
extern MidLevelRenderer::MLRClipper * theClipper;

extern GameObjectPtr DebugGameObject[3];
#ifndef FINAL
extern float CheatHitDamage;
#endif

float MissionInterfaceManager::pauseWndVelocity = -50.f;

//--------------------------------------------------------------------------------------
//class MissionInterfaceManager : public InterfaceManager
//--------------------------------------------------------------------------------------

#define CTRL	0x10000000
#define SHIFT	0x01000000
#define ALT		0x00100000
#define WAYPT	0x20000000

#define ATTILA_FACTOR			1.0f
#define ATTILA_ROTATION_FACTOR	4.1f
#define ATTILA_THRESHOLD		0.01f

int MissionInterfaceManager::mouseX = 0;
int MissionInterfaceManager::mouseY = 0;
GameObject* MissionInterfaceManager::target = NULL;
MissionInterfaceManager* MissionInterfaceManager::s_instance = NULL;
gosEnum_KeyIndex MissionInterfaceManager::s_waypointKey = (gosEnum_KeyIndex)-1;

extern bool drawTerrainTiles;
extern bool drawTerrainOverlays;
extern bool renderObjects;
extern bool drawTerrainGrid;
extern bool drawLOSGrid;
extern bool useClouds;
extern bool useFog;
extern bool useTerrainDetailTexture;
extern bool useShadows;
extern bool useFaceLighting;
extern bool useVertexLighting;
extern bool renderTGLShapes;
bool drawGUIOn = true;				//Used to shut off GUI for Screen Shots and Movie Mode
bool toggledGUI = true;				//Used to tell if I shut the GUI off or if its in movieMode
long lightState = 0;
extern bool g_dbgShowMovers;
extern bool ZeroHPrime;
extern bool CalcValidAreaTable;
extern bool g_dbgGoalPlanEnemy;
bool paintingMyVtol = false;

MissionInterfaceManager::Command		MissionInterfaceManager::s_commands[] = 
{ 
	// In-Game Controls
	"Run",						KEY_SPACE, mState_RUN, mState_RUN_LOS, false, &MissionInterfaceManager::changeSpeed, &MissionInterfaceManager::stopChangeSpeed, 43218,
	"RunWaypoint",				WAYPT | KEY_SPACE, mState_RUNWAYPT, mState_RUNWAYPT_LOS, false, &MissionInterfaceManager::changeSpeed, &MissionInterfaceManager::stopChangeSpeed, -1,
	"Jump",						KEY_J, mState_JUMP1, mState_JUMP_LOS, false, &MissionInterfaceManager::jump, &MissionInterfaceManager::stopJump, 43205,
	"JumpWaypoint",				WAYPT | KEY_J, mState_JUMPWAYPT, mState_JUMPWAYPT_LOS, false, &MissionInterfaceManager::jump, &MissionInterfaceManager::stopJump, -1,
	"AttackFromHere",			KEY_C, mState_CURPOS_ATTACK, mState_CURPOS_ATTACK_LOS, false, &MissionInterfaceManager::fireFromCurrentPos, &MissionInterfaceManager::stopFireFromCurrentPos, 43206,
	"AttackShort",				KEY_S, mState_SHRTRNG_ATTACK, mState_SHRTRNG_LOS, false, &MissionInterfaceManager::attackShort, &MissionInterfaceManager::defaultAttack, 43200,
	"AttackMedium",				KEY_M, mState_MEDRNG_ATTACK,  mState_MEDRNG_LOS, false,	&MissionInterfaceManager::attackMedium, &MissionInterfaceManager::defaultAttack, 43201,
	"AttackLong",				KEY_L, mState_LONGRNG_ATTACK, mState_LONGRNG_LOS, false, &MissionInterfaceManager::attackLong, &MissionInterfaceManager::defaultAttack, 43202,
	"AttackLegs",				KEY_NUMPAD2, mState_DONT, mState_AIMED_ATTACK_LOS, false, &MissionInterfaceManager::aimLeg, 0, 43211,
	"AttackArms",				KEY_NUMPAD5, mState_DONT, mState_AIMED_ATTACK_LOS, false, &MissionInterfaceManager::aimArm, 0, 43212,
	"AttackHead",				KEY_NUMPAD8, mState_DONT, mState_AIMED_ATTACK_LOS, false, &MissionInterfaceManager::aimHead, 0, 43213,
	"AttackEnergyOnly",			KEY_A, mState_ENERGY_WEAPONS, mState_ENERGY_WEAPONS_LOS, false, &MissionInterfaceManager::energyWeapons, &MissionInterfaceManager::energyWeapons, 43204,
	"AttackForced",				KEY_F, -1, -1, false, &MissionInterfaceManager::forceShot, 0, 43210,
	"Guard",					KEY_G, mState_GUARD, mState_GUARD, false, &MissionInterfaceManager::guard, &MissionInterfaceManager::stopGuard, 43207,
	"Stop",						KEY_BACK, -1, -1, false, &MissionInterfaceManager::removeCommand, 0, 43214,
	"PowerDown",				KEY_NEXT, -1, -1, false, &MissionInterfaceManager::powerDown, 0, 43215,
	"PowerUp",					KEY_PRIOR, -1, -1, false, &MissionInterfaceManager::powerUp, 0, 43216,
	"Eject",					KEY_END, mState_EJECT, mState_EJECT, false, &MissionInterfaceManager::eject, 0, 43217,
	"VehicleCommand",			KEY_V, -1, -1, true, &MissionInterfaceManager::vehicleCommand, 0, 43222,
	"ToggleHoldPosition",		KEY_H, -1, -1, true, &MissionInterfaceManager::toggleHoldPosition, 0, -1, // MCHD TODO: This turns off randomly; fix it
	"GoToNextNavPoint",			KEY_N, -1, -1, true, &MissionInterfaceManager::gotoNextNavMarker, 0, -1,
	"SelectVisible",			KEY_E, -1, -1, false, &MissionInterfaceManager::selectVisible, 0, 43209,
	"AddVisibleToSelection",	SHIFT | KEY_E, -1, -1, false, &MissionInterfaceManager::addVisibleToSelection, 0, -1, // MCHD TODO: Fix this so it binds with its parent key maybe?
	"DeployAirstrike",			KEY_MULTIPLY, mState_UNCERTAIN_AIRSTRIKE, mState_AIRSTRIKE, true, &MissionInterfaceManager::sendAirstrike, 0, 43220,
	"DeploySensor",				KEY_DIVIDE, mState_SENSORSTRIKE, mState_SENSORSTRIKE, true, &MissionInterfaceManager::sendSensorStrike, 0, 43221,
	
	// HUD
	"TogglePause",		KEY_ESCAPE, -1, -1, true, &MissionInterfaceManager::togglePause, 0, 43234,
	"TogglePause2",		KEY_P, -1, -1, true, &MissionInterfaceManager::togglePause, 0, -1,
	"TogglePause3",		KEY_PAUSE, -1, -1, true, &MissionInterfaceManager::togglePause, 0, -1,
	"ShowObjectives",	KEY_F9, -1, -1, true, &MissionInterfaceManager::showObjectives, 0, 43226,
	"ShowHotkeys",		KEY_F1, -1, -1, true, &MissionInterfaceManager::toggleHotKeys, 0, -1,
	"ShowCompass",		KEY_BACKSLASH, -1, -1, true, &MissionInterfaceManager::toggleCompass, 0, 43223,
	"CycleHUDTab",		KEY_TAB, -1, -1, true, &MissionInterfaceManager::switchTab, 0, -1,
	"CycleHUDTabBack",	SHIFT | KEY_TAB, -1, -1, true, &MissionInterfaceManager::reverseSwitchTab, 0, -1,
	"SelectionInfo",	KEY_I, -1, -1, true, &MissionInterfaceManager::infoCommand, &MissionInterfaceManager::infoButtonReleased, 43219,
	"AllChat",			KEY_RETURN, -1, -1, true, &MissionInterfaceManager::handleChatKey, 0, IDS_HOTKEY_CHAT,
	"TeamChat",			SHIFT | KEY_RETURN, -1, -1, true, &MissionInterfaceManager::handleTeamChatKey, 0, IDS_HOTKEY_CHAT_TEAM,

	// Camera Controls
	"CameraDefault",	KEY_HOME, -1, -1, true,	&MissionInterfaceManager::cameraNormal,0, -1,
	"CameraPreset0",	KEY_F2, -1, -1, true, &MissionInterfaceManager::cameraDefault, 0, -1,
	"CameraPreset1",	KEY_F3, -1, -1, true, &MissionInterfaceManager::cameraMaxIn, 0, -1,
	"CameraPreset2",	KEY_F4, -1, -1, true, &MissionInterfaceManager::cameraTight, 0, -1,
	"CameraPreset3",	KEY_F5, -1, -1, true, &MissionInterfaceManager::cameraFour, 0, -1,
	"CameraAssign0",	CTRL | KEY_F2, -1, -1,	true, &MissionInterfaceManager::cameraAssign0,0, -1,
	"CameraAssign1",	CTRL | KEY_F3, -1, -1, true, &MissionInterfaceManager::cameraAssign1, 0, -1,
	"CameraAssign2",	CTRL | KEY_F4, -1, -1, true, &MissionInterfaceManager::cameraAssign2, 0, -1,
	"CameraAssign3",	CTRL | KEY_F5, -1, -1, true, &MissionInterfaceManager::cameraAssign3, 0, -1,
	"CameraMoveUp",		KEY_UP,	-1,	-1,	false, &MissionInterfaceManager::scrollUp, 0,  IDS_HOTKEY_TRACKU,
	"CameraMoveDown",	KEY_DOWN, -1, -1, false, &MissionInterfaceManager::scrollDown, 0, IDS_HOTKEY_TRACKD,
	"CameraMoveLeft",	KEY_LEFT, -1, -1, false, &MissionInterfaceManager::scrollLeft, 0, IDS_HOTKEY_TRACKL,
	"CameraMoveRight",	KEY_RIGHT, -1, -1, false, &MissionInterfaceManager::scrollRight, 0, IDS_HOTKEY_TRACKR,
	"CameraLookUp",		SHIFT | KEY_UP, -1, -1, false, &MissionInterfaceManager::tiltUp, 0, IDS_HOTKEY_TILTU,
	"CameraLookDown",	SHIFT | KEY_DOWN, -1, -1, false, &MissionInterfaceManager::tiltDown, 0, IDS_HOTKEY_TILTD,
	"CameraLookLeft",	SHIFT | KEY_LEFT, -1, -1, false, &MissionInterfaceManager::rotateLeft, 0, IDS_HOTKEY_ROTATEL,
	"CameraLookRight",	SHIFT | KEY_RIGHT, -1, -1, false, &MissionInterfaceManager::rotateRight, 0, IDS_HOTKEY_ROTATER,
	"CameraZoomIn",		KEY_ADD, -1, -1, false,	&MissionInterfaceManager::zoomIn,0, 43224,
	"CameraZoomIn2",	SHIFT | KEY_EQUALS, -1, -1, false, &MissionInterfaceManager::zoomIn, 0, -1,
	"CameraZoomIn3",	KEY_EQUALS, -1, -1, false, &MissionInterfaceManager::zoomIn, 0, -1,
	"CameraZoomOut",	KEY_SUBTRACT, -1, -1, false, &MissionInterfaceManager::zoomOut, 0, 43225,
	"CameraZoomOut2",	KEY_MINUS, -1, -1, false, &MissionInterfaceManager::zoomOut, 0, -1,
};

long MissionInterfaceManager::s_numCommands = sizeof(MissionInterfaceManager::s_commands) / sizeof(MissionInterfaceManager::Command);
long MissionInterfaceManager::s_defaultKeys[sizeof(MissionInterfaceManager::s_commands) / sizeof(MissionInterfaceManager::Command)] = { };
int MissionInterfaceManager::s_chatCommandIndex = -1;

#ifndef FINAL
MissionInterfaceManager::Command		MissionInterfaceManager::s_debugCommands[] =
{
	// Objects
	"Teleport",				ALT | KEY_T, -1, -1, false, &MissionInterfaceManager::teleport, 0, -1, 
	"ToggleBrainDeadEnemy",	ALT | KEY_B, -1, -1, false, &MissionInterfaceManager::brainDead, 0, -1,
	"DamageObject0",		ALT | KEY_NUMPAD0, -1, -1, false, &MissionInterfaceManager::damageObject0, 0, -1,
	"DamageObject1",		ALT | KEY_NUMPAD1, -1, -1, false, &MissionInterfaceManager::damageObject1, 0, -1,
	"DamageObject2",		ALT | KEY_NUMPAD2, -1, -1, false, &MissionInterfaceManager::damageObject2, 0, -1,
	"DamageObject3",		ALT | KEY_NUMPAD3, -1, -1, false, &MissionInterfaceManager::damageObject3, 0, -1,
	"DamageObject4",		ALT | KEY_NUMPAD4, -1, -1, false, &MissionInterfaceManager::damageObject4, 0, -1,
	"DamageObject5",		ALT | KEY_NUMPAD5, -1, -1, false, &MissionInterfaceManager::damageObject5, 0, -1,
	"DamageObject6",		ALT | KEY_NUMPAD6, -1, -1, false, &MissionInterfaceManager::damageObject6, 0, -1,
	"GoalPlan",				ALT | KEY_P, -1, -1, false, &MissionInterfaceManager::goalPlan, 0, -1, // Still don't know what these do...
	"EnemyGoalPlan",		ALT | SHIFT | KEY_P, -1, -1, false, &MissionInterfaceManager::enemyGoalPlan, 0, -1,

	// Sun
	"DbgRotateLightUp",		ALT | CTRL | KEY_UP, -1, -1, false, &MissionInterfaceManager::rotateLightUp, 0, -1,
	"DbgRotateLightDown",	ALT | CTRL | KEY_DOWN, -1, -1, false, &MissionInterfaceManager::rotateLightDown, 0, -1,
	"DbgRotateLightLeft",	ALT | CTRL | KEY_LEFT, -1, -1, false, &MissionInterfaceManager::rotateLightLeft, 0, -1,
	"DbgRotateLightRight",	ALT | CTRL | KEY_RIGHT, -1, -1, false, &MissionInterfaceManager::rotateLightRight, 0, -1,

	// Render
	"ToggleGUI",			ALT | CTRL | KEY_Z, -1, -1, true, &MissionInterfaceManager::toggleGUI, 0, -1,
	"PerspectiveCamera",	ALT | CTRL | KEY_P, -1, -1, true, &MissionInterfaceManager::usePerspective, 0, -1,
	"DrawObjects",			ALT | CTRL | KEY_S, -1, -1, true, &MissionInterfaceManager::drawTGLShapes, 0, -1,
	"DrawBuildings",		ALT | CTRL | KEY_B, -1, -1, true, &MissionInterfaceManager::drawBuildings, 0, -1,
	"DrawTerrain",			ALT | CTRL | KEY_T, -1, -1, true, &MissionInterfaceManager::drawTerrain, 0, -1,
	"DrawOverlays",			ALT | CTRL | KEY_O, -1, -1, true, &MissionInterfaceManager::drawOverlays, 0, -1, // MCHD TODO: ???
	"DrawClouds",			ALT | CTRL | KEY_C, -1, -1, true, &MissionInterfaceManager::drawClouds, 0, -1,
	"DrawFog",				ALT | CTRL | KEY_F, -1, -1, true, &MissionInterfaceManager::drawFog, 0, -1,
	"DrawShadows",			ALT | CTRL | KEY_D, -1, -1, true, &MissionInterfaceManager::drawShadows, 0, -1,
	"DrawWaterEffects",		ALT | CTRL | KEY_V, -1, -1, true, &MissionInterfaceManager::drawWaterEffects, 0, -1,

	// Debug Render
	"ShowGrid",				ALT | CTRL | KEY_G, -1, -1, true, &MissionInterfaceManager::showGrid, 0, -1,
	"ShowLOSGrid",			ALT | CTRL | KEY_L, -1, -1, true, &MissionInterfaceManager::showLOSGrid, 0, -1,
	"ShowMovers",			ALT | KEY_M, -1, -1, false, &MissionInterfaceManager::showMovers, 0, -1,
	"ShowVictim",			ALT | KEY_V, -1, -1, false, &MissionInterfaceManager::showVictim, 0, -1, // MCHD TODO: ???
	"DisplayDebugInfo",		ALT | KEY_SLASH, -1, -1, false, &MissionInterfaceManager::quickDebugInfo, 0, -1,
	"CycleDebugWindows",	ALT | KEY_W, -1, -1, false, &MissionInterfaceManager::toggleDebugWins, 0, -1,
	"SetDebugObject",		ALT | CTRL | KEY_SLASH, -1, -1, false, &MissionInterfaceManager::setGameObjectWindow, 0, -1, // MCHD TODO: This whole thing is pretty fucked; needs some work
	"PageObjectWindow0",	ALT | CTRL | KEY_1, -1, -1, false, &MissionInterfaceManager::pageGameObjectWindow1, 0, -1,
	"PageObjectWindow1",	ALT | CTRL | KEY_2, -1, -1, false, &MissionInterfaceManager::pageGameObjectWindow2, 0, -1,
	"PageObjectWindow2",	ALT | CTRL | KEY_3, -1, -1, false, &MissionInterfaceManager::pageGameObjectWindow3, 0, -1,
	"JumpToObject0",		ALT | KEY_1, -1, -1, false, &MissionInterfaceManager::jumpToDebugGameObject1, 0, -1,
	"JumpToObject1",		ALT | KEY_2, -1, -1, false, &MissionInterfaceManager::jumpToDebugGameObject2, 0, -1,
	"JumpToObject2",		ALT | KEY_3, -1, -1, false, &MissionInterfaceManager::jumpToDebugGameObject3, 0, -1,

	// ???
	"RecalculateLights",		ALT | CTRL | KEY_Q, -1, -1, true, &MissionInterfaceManager::recalcLights, 0, -1, // MCHD TODO: Figure out what it does or remove it
	"RecalculateWater",			ALT | CTRL | KEY_W, -1, -1, true, &MissionInterfaceManager::recalcWater, 0, -1,
	"ZeroHPrime",				ALT | KEY_Z, -1, -1, false, &MissionInterfaceManager::zeroHPrime, 0, -1, // MCHD TODO: Figure out what it does or remove it
	"CalculateValidAreaTable",	ALT | KEY_A, -1, -1, false, &MissionInterfaceManager::calcValidAreaTable, 0, -1, // MCHD TODO: Figure out what it does or remove it
	"GlobalMapLog",				ALT | KEY_G, -1, -1, false, &MissionInterfaceManager::globalMapLog, 0, -1, // MCHD TODO: Figure out what it does or remove it
	"RotateObjectLeft",			ALT | KEY_PERIOD, -1, -1, false,	&MissionInterfaceManager::rotateObjectLeft, 0, -1, // MCHD TODO: Editor only?
	"RotateObjectRight",		ALT | KEY_COMMA, -1, -1, false,		&MissionInterfaceManager::rotateObjectRight, 0, -1,
};

long MissionInterfaceManager::s_numDebugCommands = sizeof(MissionInterfaceManager::s_debugCommands) / sizeof(MissionInterfaceManager::Command);
#endif

extern bool drawTerrainGrid;
extern long 	g_framesSinceMissionStart;				//What frame of the scenario is it?

extern char DebugStatusBarString[256];

void MissionInterfaceManager::init (void)
{
	realRotation = 0.0;
	dragStart.Zero();
	dragEnd.Zero();
	isDragging = FALSE;	
	
	for (long i=0;i<MAX_TEAMS;i++)
	{
		vTol[i] = 0;
		paintingVtol[i] = 0;
		vTolTime[i] = 0.0f;
		dustCloud[i] = NULL;
		recoveryBeam[i] = NULL;
		mechRecovered[i] = false;
		mechToRecover[i] = 0;
		vehicleID[i] = 0;
	}
	
	bPaused = 0;
	bPausedWithoutMenu = 0;
	resolution = Environment.screenWidth;
	s_instance = this;
	bEnergyWeapons = 0;
	zoomChoice = 2;
	memset( oldTargets, 0, sizeof( GameObject*) * MAX_ICONS );
	bDrawHotKeys = 0;
	
	hotKeyFont.init( IDS_HOT_KEY_FONT );
	hotKeyHeaderFont.init( IDS_HOT_KEY_HEADER_FONT );
	lastUpdateDoubleClick = 0;

	keyboardRef = new KeyboardRef;
	
	animationRunning = false;
	timeLeftToScroll = 0.0f;
	targetButtonId = -1;
	buttonNumFlashes = 0;
	guiFrozen = false;
	reinforcement = NULL;
	bForcedShot = false;
	bAimedShot = false;
}

#define TACMAP_ID		0
#define FLASH_JUMPERS	31798
bool MissionInterfaceManager::startAnimation(long buttonId,bool isButton,bool isPressed,float timeToScroll,long numFlashes)
{
	if (animationRunning)
		return false;
	else
	{
		animationRunning = true;
		timeLeftToScroll = timeToScroll;
		targetButtonId = buttonId;
		buttonNumFlashes = numFlashes;
		targetIsButton = isButton;
		targetIsPressed = isPressed;
		buttonFlashTime = 0.0f;

		if (buttonId < 0)		//It means flash TacMap Objective using the ABS of this objective number
		{
			controlGui.animateTacMap(buttonId,timeToScroll,numFlashes);

			animationRunning = true;
			timeLeftToScroll = timeToScroll;
			targetButtonId = TACMAP_ID;
			buttonNumFlashes = numFlashes;
			targetIsButton = false;
			targetIsPressed = false;
			buttonFlashTime = 0.0f;
		}

		if (buttonId == RPTOTAL_CALLOUT)		//RP total callout on Support Palette
		{
			controlGui.flashRPTotal(numFlashes);

			animationRunning = true;
			timeLeftToScroll = timeToScroll;
			targetButtonId = buttonId;
			buttonNumFlashes = numFlashes;
			targetIsButton = false;
			targetIsPressed = false;
			buttonFlashTime = 0.0f;
		}

		if (buttonId == FLASH_JUMPERS)		//Just flash the mech Icons.  Do not do anything else.
		{
			controlGui.forceGroupBar.flashJumpers(numFlashes);
			animationRunning = false;
		}
	}

	return true;
}

static char tutorialPlayerName[1024];
void MissionInterfaceManager::setTutorialText(const char *text)
{
	cLoadString(IDS_TUTORIAL,tutorialPlayerName,1023);
	controlGui.setChatText(tutorialPlayerName,text, 0x00ffffff, 0 );
}

void MissionInterfaceManager::update (void)
{
	if ( Environment.screenWidth != resolution )
	{
		swapResolutions();
	}

	bForcedShot = false;
	bAimedShot = false;

	if (eye && eye->inMovieMode)
	{
		drawGUIOn = false;
		userInput->mouseOff();
		
		//Need to check for ESC key so user can end IMI IMMEDIATELY.
		// After that check, just return.  Do NOT update any of the UI!!!!
		if (userInput->getKeyDown(KEY_ESCAPE))
			eye->forceMovieToEnd();

		controlGui.update(isPaused() && !isPausedWithoutMenu(), false );
			
		return;
	}
	else
	{
		drawGUIOn = toggledGUI;
		userInput->mouseOn();
	}
	
	if (animationRunning)
	{
		if (targetIsButton)
		{
			//Move mouse to correct position.
			ControlButton *targetButton = controlGui.getButton(targetButtonId);
			if (!targetButton)
			{
				animationRunning = false;
				return;
			}

			userInput->setMouseCursor(mState_TUTORIALS);

			//Get button position.
			float buttonPosX = (targetButton->location[0].x + targetButton->location[1].x +
								targetButton->location[2].x + targetButton->location[3].x) * 0.25f;

			float buttonPosY = (targetButton->location[0].y + targetButton->location[1].y +
								targetButton->location[2].y + targetButton->location[3].y) * 0.25f;

			//-------------------
			// Mouse Checks Next
			float realMouseX = userInput->realMouseX();
			float realMouseY = userInput->realMouseY();

			if (timeLeftToScroll > 0.0f)
			{
				float xDistLeft = buttonPosX - realMouseX;
				float yDistLeft = buttonPosY - realMouseY;

				float xDistThisFrame = xDistLeft / timeLeftToScroll * g_frameTime;
				float yDistThisFrame = yDistLeft / timeLeftToScroll * g_frameTime;

				userInput->setMousePos(realMouseX + xDistThisFrame, realMouseY+yDistThisFrame);

				timeLeftToScroll -= g_frameTime;
			}
			else
			{
				userInput->setMousePos(buttonPosX,buttonPosY);

				//We are there.  Start flashing.
				if (buttonNumFlashes)
				{
					buttonFlashTime += g_frameTime;
					if ( buttonFlashTime > .5f )
					{
						controlGui.getButton( targetButtonId )->setColor( 0xffffffff );
						buttonFlashTime = 0.0f;
						buttonNumFlashes--;
					}
					else if ( buttonFlashTime > .25f )
					{
						controlGui.getButton( targetButtonId )->setColor( 0xff7f7f7f );
					}
				}
				else
				{
					//Flashing is done.  We now return you to your regularly scheduled program.
					animationRunning = false;
					controlGui.getButton( targetButtonId )->setColor( 0xffffffff );
					if (targetIsPressed)
						controlGui.pushButton(targetButtonId);
				}
			}
		}
		else
		{
			ControlGui::RectInfo *targetButton = controlGui.getRect(targetButtonId);
			userInput->setMouseCursor(mState_TUTORIALS);

			//Get button position.
			float buttonPosX = (targetButton->rect.left + targetButton->rect.right) * 0.5f;
			float buttonPosY = (targetButton->rect.top + targetButton->rect.bottom) * 0.5f;

			//-------------------
			// Mouse Checks Next
			float realMouseX = userInput->realMouseX();
			float realMouseY = userInput->realMouseY();

			if (timeLeftToScroll > 0.0f)
			{
				float xDistLeft = buttonPosX - realMouseX;
				float yDistLeft = buttonPosY - realMouseY;

				float xDistThisFrame = xDistLeft / timeLeftToScroll * g_frameTime;
				float yDistThisFrame = yDistLeft / timeLeftToScroll * g_frameTime;

				userInput->setMousePos(realMouseX + xDistThisFrame, realMouseY+yDistThisFrame);

				timeLeftToScroll -= g_frameTime;
			}
			else
			{
				userInput->setMousePos(buttonPosX,buttonPosY);

				//We are there.  Start flashing.
				if (buttonNumFlashes)
				{
					buttonFlashTime += g_frameTime;
					if ( buttonFlashTime > .5f )
					{
						targetButton->color = 0xff000000;
						buttonFlashTime = 0.0f;
						buttonNumFlashes--;
					}
					else if ( buttonFlashTime > .25f )
					{
						targetButton->color = 0xffffffff;
					}
				}
				else
				{
					//Flashing is done.  We now return you to your regularly scheduled program.
					animationRunning = false;
					targetButton->color = 0xff000000;
				}
			}
		}
		
		return;		//Don't let anything else in the GUI run!!
	}
	
	//Tutorial has locked out the gui.  Move mouse to screen center and do nothing else!!
	if (guiFrozen)
	{
		userInput->setMouseCursor(mState_TUTORIALS);
		userInput->setMousePos(Environment.screenWidth * 0.5f,Environment.screenHeight * 0.5f);
		controlGui.update( isPaused() && !isPausedWithoutMenu(), false );
		return;
	}

	//---------------------------------------------------
	// Per Andy G.  One check per frame saves log file!
	bool shiftDn = userInput->shift();
	bool altDn = userInput->alt();
	bool ctrlDn = userInput->ctrl();

	//Cheats
	// Comment out here if FINAL
#ifndef FINAL
	#define MC2_DAMAGE_HEAD			0
	#define MC2_DAMAGE_LARM			1
	#define MC2_DAMAGE_RARM			2
	#define MC2_DAMAGE_LTORSO		3
	#define MC2_DAMAGE_RTORSO		4
	#define MC2_DAMAGE_CTORSO		5
	#define MC2_DAMAGE_LLEG			6
	#define MC2_DAMAGE_RLEG			7
	#define MC2_DAMAGE_RLTORSO		8
	#define MC2_DAMAGE_RRTORSO		9	
	#define MC2_DAMAGE_RCTORSO		10	
			
	DWORD CheatCommand = -1;
	if (userInput->getKeyDown(KEY_1) && shiftDn && ctrlDn && altDn)
		CheatCommand = MC2_DAMAGE_HEAD;
	if (userInput->getKeyDown(KEY_2) && shiftDn && ctrlDn && altDn)
		CheatCommand = MC2_DAMAGE_LARM;
	if (userInput->getKeyDown(KEY_3) && shiftDn && ctrlDn && altDn)
		CheatCommand = MC2_DAMAGE_RARM;
	if (userInput->getKeyDown(KEY_4) && shiftDn && ctrlDn && altDn)
		CheatCommand = MC2_DAMAGE_LTORSO;
	if (userInput->getKeyDown(KEY_5) && shiftDn && ctrlDn && altDn)
		CheatCommand = MC2_DAMAGE_RTORSO;
	if (userInput->getKeyDown(KEY_6) && shiftDn && ctrlDn && altDn)
		CheatCommand = MC2_DAMAGE_CTORSO;
	if (userInput->getKeyDown(KEY_7) && shiftDn && ctrlDn && altDn)
		CheatCommand = MC2_DAMAGE_LLEG;
	if (userInput->getKeyDown(KEY_8) && shiftDn && ctrlDn && altDn)
		CheatCommand = MC2_DAMAGE_RLEG;
	if (userInput->getKeyDown(KEY_9) && shiftDn && ctrlDn && altDn)
		CheatCommand = MC2_DAMAGE_RLTORSO;
	if (userInput->getKeyDown(KEY_0) && shiftDn && ctrlDn && altDn)
		CheatCommand = MC2_DAMAGE_RRTORSO;
	if (userInput->getKeyDown(KEY_MINUS) && shiftDn && ctrlDn && altDn)
		CheatCommand = MC2_DAMAGE_RCTORSO;

	WeaponShotInfo cheatHit;

	switch (CheatCommand)
	{
		case -1:
			break;
			
		case MC2_DAMAGE_HEAD:
			cheatHit.init(0,-2,CheatHitDamage,MECH_ARMOR_LOCATION_HEAD,0.0f);
		break;
		
		case MC2_DAMAGE_LARM:
			cheatHit.init(0,-2,CheatHitDamage,MECH_ARMOR_LOCATION_LARM,0.0f); 
		break;
		
 		case MC2_DAMAGE_RARM:
			cheatHit.init(0,-2,CheatHitDamage,MECH_ARMOR_LOCATION_RARM,0.0f); 
 		break;
		
    	case MC2_DAMAGE_LTORSO:
			cheatHit.init(0,-2,CheatHitDamage,MECH_ARMOR_LOCATION_LTORSO,0.0f); 
 		break;
		
    	case MC2_DAMAGE_RTORSO:
			cheatHit.init(0,-2,CheatHitDamage,MECH_ARMOR_LOCATION_RTORSO,0.0f); 
 		break;
		  
    	case MC2_DAMAGE_CTORSO:
			cheatHit.init(0,-2,CheatHitDamage,MECH_ARMOR_LOCATION_CTORSO,0.0f); 
 		break;
		
    	case MC2_DAMAGE_LLEG:
			cheatHit.init(0,-2,CheatHitDamage,MECH_ARMOR_LOCATION_LLEG,0.0f); 
 		break;
		
    	case MC2_DAMAGE_RLEG:
			cheatHit.init(0,-2,CheatHitDamage,MECH_ARMOR_LOCATION_RLEG,0.0f); 
 		break;
		
    	case MC2_DAMAGE_RLTORSO:
			cheatHit.init(0,-2,CheatHitDamage,MECH_ARMOR_LOCATION_RLTORSO,0.0f); 
 		break;
		
    	case MC2_DAMAGE_RRTORSO:
			cheatHit.init(0,-2,CheatHitDamage,MECH_ARMOR_LOCATION_RRTORSO,0.0f); 
 		break;
		
    	case MC2_DAMAGE_RCTORSO:
			cheatHit.init(0,-2,CheatHitDamage,MECH_ARMOR_LOCATION_RCTORSO,0.0f); 
 		break;
    }
	
	//Call handleWeaponHit for each selected Mech!
	if (CheatCommand != -1)
	{
		Team* pTeam = Team::home;
		for (long i = 0; i < pTeam->getRosterSize(); i++)
		{
			Mover* pMover = (Mover*)pTeam->getMover( i );
			if ( pMover->isSelected() && pMover->isMech())
			{
				pMover->handleWeaponHit(&cheatHit);
			}
		}
	}
#endif
	
	if ( bDrawHotKeys )
	{
		keyboardRef->update();
		return;
	}

	//-------------------
	// Mouse Checks Next
	mouseX = userInput->getMouseX();
	mouseY = userInput->getMouseY();

	bool bGui = false;

	// check and see if its in the control area
	if ( controlGui.inRegion( mouseX, mouseY, isPaused() && !isPausedWithoutMenu() ) )
	{
		bGui = true;
		if ( userInput->isLeftClick() && !userInput->isLeftDrag() )
		{
			dragStart.x = 0.f;
			dragStart.y = 0.f;
		}
	}
	else
	{
		helpTextHeaderID = helpTextID = 0;
	}
	
	if (bGui)
		userInput->setMouseCursor( mState_NORMAL );

	// find out where the mouse is
	Stuff::Vector2DOf<long>	mouseXY;
	mouseXY.x = mouseX;
	mouseXY.y = mouseY;
	eye->inverseProject(mouseXY, wPos);

	// find out if this position is passable, has line of sight
	long cellR, cellC;
	bool passable = 1;
	bool lineOfSight = 0;
	if ( Terrain::IsGameSelectTerrainPosition( wPos ) )
	{
		land->worldToCell(wPos, cellR, cellC);
		if (Team::home)	   //May go NULL during multiplayer when a player first dies?
			lineOfSight = Team::home->teamLineOfSight(wPos,0.0f);
	}	

	// update buttons and stuff, even if not in region, it draws objectives and stuff
	controlGui.update( isPaused() && !isPausedWithoutMenu(), lineOfSight );

	bool leftClicked = (!userInput->isLeftDrag() && !userInput->isRightDrag() && userInput->isLeftClick());
	bool rightClicked = (!userInput->isLeftDrag() && !userInput->wasRightDrag() && userInput->rightMouseReleased());
	bool bLeftDouble = userInput->isLeftDoubleClick();

	updateTarget(bGui);

	//------------------------------------
	// Attila (Strategic Commander) Next
	attilaXAxis 	= userInput->getAttilaXAxis();  
	attilaYAxis		= userInput->getAttilaYAxis();
	attilaRZAxis	= userInput->getAttilaRAxis();
	
	//-------------------------------
	// Update the Debug Status Bar...
	long row, col;
	land->worldToCell(wPos, row, col);
	sprintf(DebugStatusBarString, "TIME: %06d, MOUSE: [%ld, %ld] %d,%d,%d (%.2f, %.2f, %.2f), PATHMGR: %02d(%02d)",
		(long)g_missionTime,
		row, col,
		GlobalMoveMap[0]->calcArea(row, col),
		GlobalMoveMap[1]->calcArea(row, col),
		GlobalMoveMap[2]->calcArea(row, col),
		wPos.x, wPos.y, wPos.z,
		PathManager->numPaths, PathManager->peakPaths);
	if (MPlayer) {
		char mpStr[256];
		if (MPlayer->isServer())
			sprintf(mpStr, ", MULTIPLAY: %s-(%ld)SERVER {%ld,%ld}", MPlayer->getPlayerName(), MPlayer->commanderID, MPlayer->maxReceiveLoad, MPlayer->maxReceiveSize);
		else
			sprintf(mpStr, ", MULTIPLAY: %s-(%ld)CLIENT {%ld,%ld}", MPlayer->getPlayerName(), MPlayer->commanderID, MPlayer->maxReceiveLoad, MPlayer->maxReceiveSize);
		strcat(DebugStatusBarString, mpStr);
	}
	if (g_dbgGoalPlanEnemy)
		strcat(DebugStatusBarString, " [ENEMIES GOALPLAN]");
	if (g_dbgShowMovers)
		strcat(DebugStatusBarString, " [SHOW MOVERS]");
	if (ZeroHPrime)
		strcat(DebugStatusBarString, " [ZERO HPRIME]");
	if (CalcValidAreaTable)
		strcat(DebugStatusBarString, " [CALC VALID AREA]");
	if (GlobalMap::logEnabled)
		strcat(DebugStatusBarString, " [GLOBAL LOG]");
	if (!MechWarrior::brainsEnabled[1])
		strcat(DebugStatusBarString, " [BRAINDEAD: 1]");

	if ( bLeftDouble && target && target->isMover() && target->getTeam() == Team::home )
	{
		int forceGroup = -1;
		for ( int i = 0; i < 10; i++ )
		{
			if ( ((Mover*)target)->isInUnitGroup( i ) )
			{
				forceGroup = i;
				break;
			}
		}

		if ( forceGroup != -1 )
			selectForceGroup( forceGroup, true );

		lastUpdateDoubleClick = true;
	}

	Team* pTeam = Team::home;

	int moverCount = 0;
	int nonMoverCount = 0;
	for (long i=0;i<pTeam->getRosterSize();i++)
	{
		Mover* pMover = (Mover*)pTeam->getMover( i );
		if ( pMover->isSelected() && pMover->getCommander()->getId() == Commander::home->getId()  )
		{
			if ( pMover->maxMoveSpeed )
			{
				passable &= pMover->canMoveHere( wPos );
				moverCount++;
			}
			else
				nonMoverCount++;
		}
	}
	
	if( g_userPreferences.leftRightMouseProfile ) // using AOE control style
	{
		if ( s_waypointKey == -1 )
			s_waypointKey = KEY_LCONTROL;
		commandClicked = rightClicked;
		selectClicked = !bLeftDouble && !lastUpdateDoubleClick && userInput->leftMouseReleased() && !userInput->getKeyDown( KEY_T) && !isDragging;
		cameraClicked = gos_GetKeyStatus( KEY_LMENU ) == KEY_HELD;
		if ( moveCameraAround( lineOfSight, passable, ctrlDn, bGui, moverCount, nonMoverCount ) )
		{
			bool leftClicked = (!userInput->isLeftDrag() && !userInput->isRightDrag() && userInput->isLeftClick());
			bool rightClicked = (!userInput->isLeftDrag() && !userInput->wasRightDrag() && userInput->rightMouseReleased());

			// deal with the hot keys
			update( leftClicked, rightClicked, mouseX, mouseY, target,  lineOfSight );

			return;
		}

		updateAOEStyle(shiftDn, altDn, ctrlDn, bGui, lineOfSight, passable, moverCount, nonMoverCount);
	}
	else // using mc1 style
	{
		commandClicked = leftClicked;
		selectClicked = leftClicked && !lastUpdateDoubleClick;
		cameraClicked = userInput->isRightDrag();

		if ( s_waypointKey == -1 )
			s_waypointKey = KEY_LCONTROL;
		if ( moveCameraAround( lineOfSight, passable, ctrlDn, bGui, moverCount, nonMoverCount ) )
		{
			bool leftClicked = (!userInput->isLeftDrag() && !userInput->isRightDrag() && userInput->isLeftClick());
			bool rightClicked = (!userInput->isLeftDrag() && !userInput->isRightDrag() && userInput->isRightClick());

			// deal with the hot keys
			update( leftClicked, rightClicked, mouseX, mouseY, target,  lineOfSight );
			return;
		}

		updateOldStyle(shiftDn, altDn, ctrlDn, bGui, lineOfSight, passable, moverCount, nonMoverCount);
	}

	for ( i = 0; i < Team::home->getRosterSize(); i++ )
	{
		Mover* pMover = (Mover*)Team::home->getMover( i );
		MechWarrior* pilot = pMover->getPilot();
		if ( pilot && pMover->getCommander()->getId() == Commander::home->getId())
		{
			GameObject* pTmpTarget = pilot->getCurrentTarget( );
			if ( pTmpTarget && (i < MAX_ICONS))	//Must check this because old test maps have more then 16 movers on them!!
			{
				pTmpTarget->setDrawBars(true);
				oldTargets[i] = pTmpTarget;
			}
		}
	}

	if ( !bLeftDouble && !( lastUpdateDoubleClick && 
		userInput->getMouseLeftButtonState() == MC2_MOUSE_DOWN ) )// check for the hold )
		lastUpdateDoubleClick = false;

	updateRollovers();
}

void MissionInterfaceManager::updateVTol()
{
	//We're probably going to use this alot!
	long commanderID = Commander::home->getId();
	paintingMyVtol = paintingVtol[commanderID];
	
	for (long vtolNum = 0;vtolNum < MAX_TEAMS; vtolNum++)
	{
		// update the vtol, if it needs it
		if ( paintingVtol[vtolNum] && (!bPaused || MPlayer))
		{
			if (vehicleID[vtolNum] != 147)		//We are the standard vtol.
			{
				if (!vehicleDropped[vtolNum] && (vTol[vtolNum]->currentFrame >= 145))
				{
					vehicleDropped[vtolNum] = true;
					g_soundSystem->playDigitalSample(VTOL_DROP,vPos[vtolNum]);
					
					//OK, if this is another persons vtol, I probably shouldn't do this
					// Glenn, what should it do?
					// Just check my commanderID against the vtolNum and if they match do it?
					// Otherwise do nothing?
					// Help Me, Spock!
					// -fs
					if (MPlayer) 
					{
						if (commanderID == vtolNum)
						{
							//--------------------------------------------------------
							// This will get the other machines to enable the mover...
							MPlayer->sendReinforcement(vehicleID[vtolNum], MPlayer->reinforcements[commanderID][0], "noname", commanderID, vPos[vtolNum], 2);
						}
					}
					else
						addVehicle( vPos[vtolNum] );
				}
				else if (vTol[vtolNum]->currentFrame >= 300)
				{
					paintingVtol[vtolNum] = 0;
					delete vTol[vtolNum]; 
					vTol[vtolNum] = NULL;
					
					if (dustCloud[vtolNum])
					{
						dustCloud[vtolNum]->Kill();
						delete dustCloud[vtolNum];
						dustCloud[vtolNum] = NULL;
					}
					
					if (recoveryBeam[vtolNum])
					{
						recoveryBeam[vtolNum]->Kill();
						delete recoveryBeam[vtolNum];
						recoveryBeam[vtolNum] = NULL;
					}

					//We should have set this when the helo was brought in.
					// Let it takeoff now that the vtol is out of the way.
					if (MPlayer) {
						if ((vtolNum < 0) || (vtolNum >= MAX_MC_PLAYERS))
							STOP(("updateVTOL: bad vtolNum"));
						if (MPlayer->reinforcements[vtolNum][0] < 0)
							STOP(("updateVTOL: bad reinforcement"));
						MoverPtr mover = MPlayer->moverRoster[MPlayer->reinforcements[vtolNum][0]];
						if (mover) {
							TacticalOrder tacOrder;
							tacOrder.init(ORDER_ORIGIN_PLAYER, TACTICAL_ORDER_POWERUP, true);
							tacOrder.pack(NULL, NULL);					
							if (!MPlayer->isServer())
								MPlayer->sendPlayerOrder(&tacOrder, false, 1, &mover);
							else
								mover->handleTacticalOrder(tacOrder);
						}
						MPlayer->reinforcements[vtolNum][0] = -1;
						}
					else if (reinforcement)
					{
						reinforcement->getPilot()->orderPowerUp(true, ORDER_ORIGIN_SELF);
						reinforcement = NULL;
					}

					if ( !MPlayer || vtolNum == MPlayer->commanderID )
						controlGui.unPressAllVehicleButtons();
				}
			}
			else	//Update the KARNOV animation
			{
				if ((vTol[vtolNum]->getCurrentGestureId() == 1) && !vTol[vtolNum]->getInTransition())
				{
					vTol[vtolNum]->setGesture(0);

					//Needs to STAY false, probably in MPlayer too because
					// If the mech we are about to recover gets destroyed, we need to abort!!
					// Probably in Multiplayer now its possible to recover even if destroyed!
					if (!MPlayer)
						mechRecovered[vtolNum] = false;		//Need to know when mech is done so Karnov can fly away.

					vTolTime[vtolNum] = g_missionTime;
					
					//Start GOsFX - We are now hovering.
					if (recoveryBeam[vtolNum])
					{
						Stuff::LinearMatrix4D 	shapeOrigin;
						Stuff::LinearMatrix4D	localToWorld;
						Stuff::LinearMatrix4D	localResult;
									
						Stuff::Vector3D dustPos = vTol[vtolNum]->position;
						Stuff::Point3D wakePos;
						wakePos.x = -dustPos.x;
						wakePos.y = dustPos.z;
						wakePos.z = dustPos.y;
							
						shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
						shapeOrigin.BuildTranslation(wakePos);
									
						gosFX::Effect::ExecuteInfo info((Stuff::Time)g_missionTime,&shapeOrigin,NULL);
					
						recoveryBeam[vtolNum]->SetLoopOff();
						recoveryBeam[vtolNum]->SetExecuteOn();
						recoveryBeam[vtolNum]->Start(&info);
					}

					//Actually the Karnov Deploy Sound Effect!
					g_soundSystem->playDigitalSample(RADAR_HUM,vTol[vtolNum]->position,false);
				}
				else if ((vTol[vtolNum]->getCurrentGestureId() == 0) && !mechRecovered[vtolNum] && mechToRecover[vtolNum] && !mechToRecover[vtolNum]->isDestroyed())
				{
					if (g_missionTime > vTolTime[vtolNum] + Mover::recoverTime)
					{
						bool doIt = true;
						if (MPlayer)
						{
							if (commanderID == vtolNum)
							{
								//--------------------------------------------------------
								// This will get the other machines to enable the mover...
								MPlayer->sendReinforcement(vehicleID[vtolNum], MPlayer->reinforcements[vtolNum][1], MPlayer->reinforcementPilot[vtolNum], vtolNum, vPos[vtolNum], 5);
							}
							else
								doIt = false;
						}
						
						if (doIt && !mechToRecover[vtolNum]->isDestroyed())
						{
							//Seems like we don't want to do this on every machine, do we?
							// -fs

							//STAY in recover until its done.  I know it shouldn't be possible but trust me, its happening.
							while (((MoverPtr)mechToRecover[vtolNum])->recover() == false)
								;

							if (mechToRecover[vtolNum]->isDisabled())
							{
								mechToRecover[vtolNum]->setStatus(OBJECT_STATUS_SHUTDOWN,true);
								mechToRecover[vtolNum]->getSensorSystem()->broken = false;
								((MoverPtr)mechToRecover[vtolNum])->timeLeft = 1.0f;
								((MoverPtr)mechToRecover[vtolNum])->exploding = false;
							}
							char* newPilotName = NULL;
							if (MPlayer)
								newPilotName = MPlayer->reinforcementPilot[vtolNum];
							else
								newPilotName = (char*)LogisticsData::instance->getBestPilot( mechToRecover[vtolNum]->tonnage );

							mission->tradeMover(mechToRecover[vtolNum], Commander::commanders[vtolNum]->getTeam()->getId(), vtolNum, newPilotName, "pbrain");
							mechRecovered[vtolNum] = true;
							mechToRecover[vtolNum]->getPilot()->orderPowerUp(true, ORDER_ORIGIN_SELF);
							if (MPlayer)
								MPlayer->reinforcements[vtolNum][1] = -1;
						}
					}
				}
				else if ((vTol[vtolNum]->getCurrentGestureId() == 0) && (mechRecovered[vtolNum] || mechToRecover[vtolNum]->isDestroyed()) && (!vTol[vtolNum]->getInTransition()))
				{
					vTol[vtolNum]->setGesture(2);	//Fly Away!!
				}
				else if ((vTol[vtolNum]->getCurrentGestureId() == 2) && (!vTol[vtolNum]->getInTransition()))
				{
					paintingVtol[vtolNum] = 0;
					delete vTol[vtolNum];
					vTol[vtolNum] = NULL;
					
					if (dustCloud[vtolNum])
					{
						dustCloud[vtolNum]->Kill();
						delete dustCloud[vtolNum];
						dustCloud[vtolNum] = NULL;
					}
					
					if (recoveryBeam[vtolNum])
					{
						recoveryBeam[vtolNum]->Kill();
						delete recoveryBeam[vtolNum];
						recoveryBeam[vtolNum] = NULL;
					}

					//Check if mech was recovered.  If not, restore the resource points!!!
					// This can happen if we call in a recovery vehicle and the mech is destroyed
					// BEFORE the recovery vehicle recovers it!!
					if (!mechRecovered[vtolNum]) {
						if (MPlayer) {
							if (MPlayer->isServer()) {
								//MPlayer->playerInfo[vtolNum].resourcePoints += 10000;
								Stuff::Vector3D pos;
								MPlayer->sendReinforcement(10000, 0, "noname", vtolNum, pos, 6);
							}
							}
						else
							LogisticsData::instance->setResourcePoints(LogisticsData::instance->getResourcePoints() + 10000);
					}

					if ( !MPlayer || vtolNum == MPlayer->commanderID )
						controlGui.unPressAllVehicleButtons();
				}
			}
	
			if (dustCloud[vtolNum] && dustCloud[vtolNum]->IsExecuted())
			{
				Stuff::LinearMatrix4D 	shapeOrigin;
				Stuff::LinearMatrix4D	localToWorld;
				Stuff::LinearMatrix4D	localResult;
						
				Stuff::Vector3D dustPos = vTol[vtolNum]->position;
				Stuff::Point3D wakePos;
				wakePos.x = -dustPos.x;
				wakePos.y = dustPos.z;
				wakePos.z = dustPos.y;
				
				shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
				shapeOrigin.BuildTranslation(wakePos);
						
				Stuff::OBB boundingBox;
				gosFX::Effect::ExecuteInfo info((Stuff::Time)g_missionTime,&shapeOrigin,&boundingBox);
				
				dustCloud[vtolNum]->Execute(&info);
			}
			
			if (recoveryBeam[vtolNum] && recoveryBeam[vtolNum]->IsExecuted())
			{
				Stuff::LinearMatrix4D 	shapeOrigin;
				Stuff::LinearMatrix4D	localToWorld;
				Stuff::LinearMatrix4D	localResult;
						
				Stuff::Vector3D dustPos = vTol[vtolNum]->position;
				Stuff::Point3D wakePos;
				wakePos.x = -dustPos.x;
				wakePos.y = dustPos.z;
				wakePos.z = dustPos.y;
				
				shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
				shapeOrigin.BuildTranslation(wakePos);
						
				Stuff::OBB boundingBox;
				gosFX::Effect::ExecuteInfo info((Stuff::Time)g_missionTime,&shapeOrigin,&boundingBox);
				
				recoveryBeam[vtolNum]->Execute(&info);
			}
				
			if (vTol[vtolNum])	//Might have been deleted above!!
			{
				vTol[vtolNum]->recalcBounds();
				vTol[vtolNum]->update();			//Must update even if not on screen!
			}
		}
		else if (paintingVtol[vtolNum] && bPaused)
		{
			if (vTol[vtolNum])	//Might have been deleted above!!
			{
				long cFrame = vTol[vtolNum]->currentFrame;
				
				vTol[vtolNum]->recalcBounds();
				vTol[vtolNum]->update();			//Must update even if not on screen!
				
				//If we are paused, do not animate.
				vTol[vtolNum]->currentFrame = cFrame;
			}
		}
	}
}

void MissionInterfaceManager::updateTarget( bool bGui)
{
	
	// unset anything that isn't targeted
	for ( int i = 0; i < MAX_ICONS; i++ )
	{
		if ( oldTargets[i] )
		{
			oldTargets[i]->setDrawBars( 0 );
			oldTargets[i] = 0;
		}
	}
	
	// if there was a target, unset it
	if ( target )
		target->setTargeted( 0 );			

	//----------------------------------------------------------------------------------------
	// Get Any object we are over.  Change cursor if appropriate.  Set selected if necessary.
	// Clear the target pointer if we have done all we need to do here.  Otherwise, leave it
	// set and issue an order below based on who is in the myForce pointers.
	target = ObjectManager->findObjectByMouse(mouseX, mouseY);
	if ( target )
	{
		if ( bGui )
			target = 0;
		else if ( target->isMover() && !g_dbgShowMovers && !(MPlayer && MPlayer->allUnitsDestroyed[MPlayer->commanderID]))
		{
			if ((target->getTeamId() != Team::home->getId()) && 
				!target->isDisabled() && 
				(((Mover *)target)->conStat < CONTACT_SENSOR_QUALITY_1))
				target = NULL;
		}
		
		if ( target )
		{
			int descID = target->getDescription();
			if ( descID != -1 )
			{
				int nameID = target->getAppearance()->getObjectNameId();

				helpTextHeaderID = nameID;
				helpTextID = descID;
			}
			if ( target->isDestroyed() )
			{
				target = 0;
			}
			
			else if ( target->isDisabled() && !target->isMover() )
				target = 0;
			else if ( !target->isSelectable())
			{
				if ( !target->isDisabled() && !target->isMover() )
					target = NULL;
			}
		}
	}
}

void MissionInterfaceManager::drawWayPointPaths()
{
	Team* pTeam = Team::home;
   	for (long i=0;i<pTeam->getRosterSize();i++)
   	{
   		Mover* pMover = (Mover*)pTeam->getMover( i );
   		if ( pMover->isSelected() && pMover->getCommander()->getId() == Commander::home->getId() )
   		{
   			pMover->updateDrawWaypointPath();
   		}
   	}
}
	
void MissionInterfaceManager::updateOldStyle( bool shiftDn, bool altDn, bool ctrlDn, 
											 bool bGui, bool lineOfSight, bool passable, 
											 long moverCount, long nonMoverCount )
{

	printDebugInfo();

	//We're probably going to use this alot!
	long commanderID = Commander::home->getId();
	
 	// Update the waypoint markers so that they are visible!
   	if ( userInput->getKeyDown( s_waypointKey ) || controlGui.getMines() )
   	{
		drawWayPointPaths(); 	
		//Can Never make a patrol path.  Causes movement wubbies!
//		if ( makePatrolPath() )
//			return;
   	}

	int mState = userInput->getMouseCursor();

	bool leftClicked = (!userInput->isLeftDrag() && !userInput->isRightDrag() && userInput->isLeftClick() && !lastUpdateDoubleClick);
	bool rightClicked = (!userInput->isLeftDrag() && !userInput->isRightDrag() && userInput->isRightClick());

	// deal with the hot keys
	if ( update( leftClicked, rightClicked, mouseX, mouseY, target,  lineOfSight ) )
		return;

	// special case for bottom of screen
	if ( bGui ) // don't target or anything if we are int the control panel
	{
		// now check the selection versus the commands
		if ( !canJump() && controlGui.getJump() )
			controlGui.toggleJump();

		userInput->setMouseCursor( mState );
		if ( controlGui.mouseInVehicleStopButton )
			return;
		if ( !userInput->isLeftDrag() && !controlGui.isAddingAirstrike() && !controlGui.isAddingVehicle() )
			return;
	}

	Team* pTeam = Team::home;
	
	if (target) // if there is a target, make the appropritate cursor
	{
		userInput->setMouseCursor( makeTargetCursor( lineOfSight, moverCount, nonMoverCount ) );
	}
	else // make a move cursor
	{
		userInput->setMouseCursor( makeNoTargetCursor( passable, lineOfSight, ctrlDn, bGui, moverCount, nonMoverCount ) );
	}

	if ( userInput->leftMouseReleased() && !userInput->wasLeftDrag() && !bGui && !lastUpdateDoubleClick) // move on the mouse ups
	{

		if ( (controlGui.isAddingVehicle() && !paintingVtol[commanderID] && canAddVehicle( wPos )) || 
			(controlGui.isAddingSalvage() && !paintingVtol[commanderID] && canRecover( wPos )))
		{
			// Target has already been confirmed  as a mover(BattleMech) by canRecover
			// OR it doesn't matter because the canAddVehicle part of the beginVtol function doesn't reference it!
			// Again, Its OK because we only use it for salvage craft.
			// Need to pass this in so that Multiplayer can pass it in.
			beginVtol(-1,commanderID,NULL,(MoverPtr)target);	//In Single player, this should always be zero?
			return;
		}
		else if ( (controlGui.isAddingVehicle() && !paintingVtol[commanderID] && !canAddVehicle( wPos )) || 
				(controlGui.isAddingSalvage() && !paintingVtol[commanderID] && !canRecover( wPos )))
		{
			g_soundSystem->playDigitalSample( INVALID_GUI );
			return;
		}
		else if ( controlGui.isAddingAirstrike() && !paintingVtol[commanderID] ) // if we're painting the vtol, carry on
		{
			addAirstrike();
			return;
		}
		
		else if (!target)
		{
			if ( controlGui.isSelectingInfoObject() )
			{
				g_soundSystem->playDigitalSample( INVALID_GUI );
				controlGui.cancelInfo();
			}
			else if (Terrain::IsGameSelectTerrainPosition(wPos))
			{
				
				if (controlGui.getGuardTower())
					doGuardTower();
				else if ( passable || selectionIsHelicopters())
				{
					if ( controlGui.getGuard() )
					{
						doGuard(target);
					}
					else if (!bForcedShot)
						doMove( wPos );
				}
				else
				{
					userInput->setMouseCursor(mState_DONT);
				}
			}
			else
			{
				userInput->setMouseCursor(mState_DONT);
			}
		}
		else 
			//We clicked on a target.  If mouse cursor is normal, we want to select a friendly.
				//if the mouse cursor is some form of attack, attack the target.
		{
			if ( target->isMover() && ( target->getTeamId() == Team::home->getId() || 
					CONTACT_VISUAL == ((Mover*)target)->getContactStatus(Team::home->getId(), true) 
					|| target->isDisabled() ) )
				{
					controlGui.setInfoWndMover( (Mover*)target );
				}
		
			if ( controlGui.getSalvage() )
				doSalvage();
			else if ( controlGui.getGuard() )
			{
				doGuard(target);
			}
			else if ( target->getTeamId() == Team::home->getId() && !target->isDisabled() )
			{
				
				//--------------------------------------------------------------------
				// User wants to select the guy we are on.  If he did
				// it with a shift, add him.  If not, whack all and just add target!
				if (shiftDn && target->getCommanderId() == Commander::home->getId() )
				{
					bool alreadyThere = false;
					//-------------------------------------------
					// First, check if he's already there!
					for (long i=0;i<pTeam->getRosterSize();i++)
					{
						Mover* pMover = (Mover*)pTeam->getMover( i );
						if (pMover == target && pMover->isSelected() && pMover->getCommander()->getId() == Commander::home->getId())
							alreadyThere = true;
					}
					
					if (!alreadyThere && !target->isDisabled())
						target->setSelected( true );
					else
						target->setSelected( false );
				}
				else if ( controlGui.getRepair() && canRepair( target ) )
				{
					doRepair(target);
				}
			 	else if ( controlGui.getSalvage() && canSalvage( target ))
					doSalvage();
				else if ( controlGui.getGuardTower() )
					doGuardTower();
				else if ( target->getTeam() && Team::home->isFriendly(target->getTeam()) && target->getFlag(OBJECT_FLAG_CANREFIT) && target->getFlag(OBJECT_FLAG_MECHBAY) && canRepairBay(target) )
					doRepairBay(target);
				else if ( controlGui.isSelectingInfoObject() && target->isMover() )
					controlGui.setInfoWndMover( (Mover*)target );
				else if ( target->isMover() && !target->isDisabled() && target->getCommanderId() == Commander::home->getId() )
				{
					for (long i=0;i<pTeam->getRosterSize();i++)
					{
						Mover* pMover = (Mover*)pTeam->getMover( i );
						if (pMover->getCommander()->getId() == Commander::home->getId())
						{
							pMover->setSelected( false );
						}
					}
					
					target->setSelected( true );
				}
				else
					g_soundSystem->playDigitalSample( INVALID_GUI );
			}
			else
			{
				if ( controlGui.getGuard() ) // trying to guard invalid thing, cancel guard
					controlGui.toggleGuard();
				else if ( userInput->getMouseCursor() == mState_INFO && 
						target->isMover()  )
							controlGui.setInfoWndMover( (Mover*)target );
				else if ( !target->isDisabled() && !bForcedShot && !bAimedShot)
					doAttack();
			}
		}
	}
}	
void MissionInterfaceManager::updateAOEStyle(bool shiftDn, bool altDn, bool ctrlDn, 
											  bool bGui, bool lineOfSight, bool passable, 
											  long moverCount, long nonMoverCount )
{
	printDebugInfo();

	//We're probably going to use this alot!
	long commanderID = Commander::home->getId();

 	// Update the waypoint markers so that they are visible!
   	if ( userInput->getKeyDown( s_waypointKey ) || controlGui.getMines() )
   	{
		drawWayPointPaths(); 	
		//Can Never make a patrol path.  Causes movement wubbies!
//		if ( makePatrolPath() )
//			return;
   	}

	int mState = userInput->getMouseCursor();

	bool leftClicked = (!userInput->isLeftDrag() && !userInput->isRightDrag() && userInput->isLeftClick());
	bool rightClicked = (!userInput->isLeftDrag() && !userInput->wasRightDrag() && userInput->rightMouseReleased());

	// deal with the hot keys
	if ( update( leftClicked, rightClicked, mouseX, mouseY, target,  lineOfSight ) )
		return;

	// special case for bottom of screen
	if ( bGui ) // don't target or anything if we are int the control panel
	{
		// now check the selection versus the commands
		if ( !canJump() && controlGui.getJump() )
			controlGui.toggleJump();

		userInput->setMouseCursor( mState );
		if ( controlGui.mouseInVehicleStopButton )
			return;
		if ( !userInput->isLeftDrag() && !controlGui.isAddingAirstrike() && !controlGui.isAddingVehicle() && !controlGui.isAddingSalvage() )
			return;
	}
	
	Team* pTeam = Team::home;
	
	if (target) // if there is a target, make the appropritate cursor
	{
		userInput->setMouseCursor( makeTargetCursor( lineOfSight, moverCount, nonMoverCount ) );
	}
	else // make a move cursor
	{
		userInput->setMouseCursor( makeNoTargetCursor( passable, lineOfSight, ctrlDn, bGui, moverCount, nonMoverCount ) );
	}

	if ( userInput->rightMouseReleased() && !userInput->wasRightDrag() && !bGui) // move on the mouse ups
	{

		if ( (controlGui.isAddingVehicle() && !paintingVtol[commanderID] && canAddVehicle( wPos )) || 
			(controlGui.isAddingSalvage() && !paintingVtol[commanderID] && canRecover( wPos )))
		{
			// Target has already been confirmed  as a mover(BattleMech) by canRecover
			// OR it doesn't matter because the canAddVehicle part of the beginVtol function doesn't reference it!
			// Again, Its OK because we only use it for salvage craft.
			// Need to pass this in so that Multiplayer can pass it in.
			beginVtol(-1,commanderID,NULL,(MoverPtr)target);	//In Single player, this should always be zero?
			return;
		}
		else if ( (controlGui.isAddingVehicle() && !paintingVtol[commanderID] && !canAddVehicle( wPos )) || 
				(controlGui.isAddingSalvage() && !paintingVtol[commanderID] && !canRecover( wPos )))
		{
			g_soundSystem->playDigitalSample( INVALID_GUI );
			return;
		}
		else if ( controlGui.isAddingAirstrike() && !paintingVtol[commanderID] ) // if we're painting the vtol, carry on
		{
			addAirstrike();
			return;
		}
		
		else if (!target )
		{
			if ( controlGui.isSelectingInfoObject() )
			{
				g_soundSystem->playDigitalSample( INVALID_GUI );
				controlGui.cancelInfo();
			}
			else if (Terrain::IsGameSelectTerrainPosition(wPos))
			{
				if ( passable || selectionIsHelicopters() )
				{
					if (controlGui.getGuardTower())
						doGuardTower();
					else if ( controlGui.getGuard() )
					{
						doGuard(NULL);
					}
					else				
						doMove( wPos );
				}
				else
				{
					userInput->setMouseCursor(mState_DONT);
				}
			}
			else
			{
				userInput->setMouseCursor(mState_DONT);
			}
		} 
		else if ( controlGui.getSalvage() )
			doSalvage();
		else if ( controlGui.getGuard() )
			doGuard(target);
		else if ( controlGui.getRepair() && canRepair( target ) )
			doRepair(target);
		else if ( controlGui.getGuardTower() )
			doGuardTower();
		else if ( target->getTeam() && Team::home->isFriendly(target->getTeam()) && target->getFlag(OBJECT_FLAG_CANREFIT) && target->getFlag(OBJECT_FLAG_MECHBAY) && canRepairBay(target) )
			doRepairBay(target);
		else if ( controlGui.getSalvage() && canSalvage( target ))
			doSalvage();
		else if ( controlGui.isSelectingInfoObject() && target->isMover() )
			controlGui.setInfoWndMover( (Mover*)target );
		else if ( target->getTeam() != Team::home && !target->isDisabled() )
			doAttack();
		else if (!bForcedShot && !bAimedShot)
			g_soundSystem->playDigitalSample( INVALID_GUI );
	}
	else if ( selectClicked && !bGui )
	{
			//We clicked on a target.  If mouse cursor is normal, we want to select a friendly.
				//if the mouse cursor is some form of attack, attack the target.
		if ( (controlGui.isAddingVehicle() && !paintingVtol[commanderID] && canAddVehicle( wPos )) || 
			(controlGui.isAddingSalvage() && !paintingVtol[commanderID] && canRecover( wPos )))
		{
			// Target has already been confirmed  as a mover(BattleMech) by canRecover.
			// OR it doesn't matter because the canAddVehicle part of the beginVtol function doesn't reference it!
			// Again, Its OK because we only use it for salvage craft.
			// Need to pass this in so that Multiplayer can pass it in.
			beginVtol(-1,commanderID,NULL,(MoverPtr)target);	//In Single player, this should always be zero?
			return;
		}
		else if ( (controlGui.isAddingVehicle() && !paintingVtol[commanderID] && !canAddVehicle( wPos )) || 
				(controlGui.isAddingSalvage() && !paintingVtol[commanderID] && !canRecover( wPos )))
		{
			g_soundSystem->playDigitalSample( INVALID_GUI );
			return;
		}
		else if ( controlGui.isAddingAirstrike() && !paintingVtol[commanderID] ) // if we're painting the vtol, carry on
		{
			addAirstrike();
			return;
		}

		else if ( target )
		{
			if ( target->isMover() && ( target->getTeamId() == Team::home->getId() || 
					CONTACT_VISUAL == ((Mover*)target)->getContactStatus(Team::home->getId(), true) 
					|| target->isDisabled() ) )
				{
					controlGui.setInfoWndMover( (Mover*)target );
				}
		
			if (target->getCommanderId() == Commander::home->getId() && !target->isDisabled()  )
			{
				
				//--------------------------------------------------------------------
				// User wants to select the guy we are on.  If he did
				// it with a shift, add him.  If not, whack all and just add target!
				if (shiftDn)
				{
					bool alreadyThere = false;
					//-------------------------------------------
					// First, check if he's already there!
					for (long i=0;i<pTeam->getRosterSize();i++)
					{
						Mover* pMover = (Mover*)pTeam->getMover( i );
						if (pMover == target && pMover->isSelected() && pMover->getCommander()->getId() == Commander::home->getId() )
							alreadyThere = true;
					}
					
					if (!alreadyThere && !target->isDisabled())
						target->setSelected( true );
					else
						target->setSelected( false );
				}
				else if ( target->isMover() )
				{
					for (long i=0;i<pTeam->getRosterSize();i++)
					{
						Mover* pMover = (Mover*)pTeam->getMover( i );
						if (pMover->getCommander()->getId() == Commander::home->getId())
						{
							pMover->setSelected( false );
						}
					}
					
					target->setSelected( true );
				}
			}			
		}
		else
		{	
			// tried to select nothing
			controlGui.cancelInfo();
			g_soundSystem->playDigitalSample( INVALID_GUI );
		}
	}
}

void MissionInterfaceManager::updateWaypoints (void)
{
	// Update the waypoint markers so that they are visible and must happen AFTER camera update!!
	// Or they wiggle something fierce.
   	if ( userInput->getKeyDown( s_waypointKey ) || controlGui.getMines() )
   	{
   		Team* pTeam = Team::home;
   		for (long i=0;i<pTeam->getRosterSize();i++)
   		{
   			Mover* pMover = (Mover*)pTeam->getMover( i );
   			if ( pMover->isSelected() && pMover->getCommander()->getId() == Commander::home->getId() )
   			{
   				pMover->updateDrawWaypointPath();
   			}
   		}
   	}
}

int MissionInterfaceManager::update( bool leftClickedClick, bool rightClickedClick, int MouseX, int MouseY, GameObject* pTarget, bool bLOS )
{
	bool shiftDn = userInput->shift();
	bool ctrlDn = userInput->ctrl();
	bool altDn = userInput->alt();
	bool wayPtDn = userInput->getKeyDown( s_waypointKey );

	bool bRetVal = 0;

	int i = 0;
	int numCommands = s_numCommands;

	// if chatting, ignore keyboard input except for the two chat buttons
	if ( controlGui.updateChat() )
	{
		i = s_chatCommandIndex;;
		numCommands = i + 2;
		bRetVal = 1;
	}

	if ( gos_GetKeyStatus( s_waypointKey ) == KEY_RELEASED )
	{
		Team* pTeam  = Team::home;
		for (long i = 0; i < pTeam->getRosterSize(); i++)
		{
			Mover* pMover = (Mover*)pTeam->getMover( i );
			if ( pMover->isSelected() && !pMover->getPilot()->getExecutingTacOrderQueue() && pMover->getCommander()->getId() == Commander::home->getId() )
			{
				if (MPlayer && !MPlayer->isServer()) {
					TacticalOrder tacOrder;
					tacOrder.init(ORDER_ORIGIN_PLAYER, TACTICAL_ORDER_WAYPOINTS_DONE);
					tacOrder.pack(NULL, NULL);					
					MPlayer->sendPlayerOrder(&tacOrder, false, 1, &pMover);
					}
				else {
					pMover->getPilot()->setExecutingQueue(true);
					pMover->getPilot()->executeTacOrderQueue();
				}
			}
		}
		bRetVal = 1;
	}

	mouseX = MouseX;
	mouseY = MouseY;
	setTarget( pTarget );

	if ( !bRetVal )
	{
		for ( int j = KEY_0; j < KEY_9 + 1; j++ )
		{
			if ( userInput->getKeyDown( (gosEnum_KeyIndex)j )
				&& gos_GetKeyStatus( (gosEnum_KeyIndex)j ) != KEY_HELD )
			{
				if ( ctrlDn )
					makeForceGroup( j - KEY_0 );
				else if ( shiftDn )
					selectForceGroup( j - KEY_0, 0 );
				else
					selectForceGroup( j - KEY_0, 1 );
			}
		}
	}

	for (; i < numCommands; i++)
	{
		int key = s_commands[i].key;
		if ( userInput->getKeyDown( gosEnum_KeyIndex(key & 0x0000000ff) ) )
		{
			// check for shifts and stuff
			// must check way pt first, because it can be shift or ctrl
			if ( (key & WAYPT) )
			{	if ( !wayPtDn )
   					continue;
			}
			else if ( ((key & SHIFT) ? true : false) != shiftDn )
				continue;
			else if ( ((key & CTRL) ? true : false) != ctrlDn )
				continue;
			else if ( ((key & ALT) ? true : false) != altDn )
				continue;
			
			// got this far, call the command
			if ( s_commands[i].key != -1 )
				userInput->setMouseCursor(bLOS ? s_commands[i].cursorLOS : s_commands[i].cursor);

			if ( !s_commands[i].singleClick )
			{
				if ( (this->*s_commands[i].function)() )
					bRetVal = 1;
			}
			else if ( gos_GetKeyStatus( (gosEnum_KeyIndex)(key & 0x000fffff) ) != KEY_HELD )
			{
				if ( this->s_commands[i].function && (this->*s_commands[i].function)() )
					bRetVal = 1;
			}	
		}
		else if ( gos_GetKeyStatus( gosEnum_KeyIndex(key & 0x000fffff) ) == KEY_RELEASED 
			&& s_commands[i].releaseFunction )
		{
			if ( (this->*s_commands[i].releaseFunction)() )
				bRetVal = 1;
		}
	}

#ifndef FINAL
	for (int i = 0; i < s_numDebugCommands; ++i)
	{
		int key = s_debugCommands[i].key;
		if (userInput->getKeyDown(gosEnum_KeyIndex(key & 0x0000000ff)))
		{
			// check for shifts and stuff
			// must check way pt first, because it can be shift or ctrl
			if ((key & WAYPT))
			{
				if (!wayPtDn)
					continue;
			}
			else if (((key & SHIFT) ? true : false) != shiftDn)
				continue;
			else if (((key & CTRL) ? true : false) != ctrlDn)
				continue;
			else if (((key & ALT) ? true : false) != altDn)
				continue;

			// got this far, call the command
			if (s_debugCommands[i].key != -1)
				userInput->setMouseCursor(bLOS ? s_debugCommands[i].cursorLOS : s_debugCommands[i].cursor);

			if (!s_debugCommands[i].singleClick)
			{
				if ((this->*s_debugCommands[i].function)())
					bRetVal = 1;
			}
			else if (gos_GetKeyStatus((gosEnum_KeyIndex)(key & 0x000fffff)) != KEY_HELD)
			{
				if (this->s_debugCommands[i].function && (this->*s_debugCommands[i].function)())
					bRetVal = 1;
			}
		}
		else if (gos_GetKeyStatus(gosEnum_KeyIndex(key & 0x000fffff)) == KEY_RELEASED
			&& s_debugCommands[i].releaseFunction)
		{
			if ((this->*s_debugCommands[i].releaseFunction)())
				bRetVal = 1;
		}
	}
#endif

	return bRetVal;
}

void MissionInterfaceManager::doAttack()
{

	if ( userInput->getKeyDown( s_waypointKey ) || !target )
	{
		g_soundSystem->playDigitalSample( INVALID_GUI );
		return; // don't do if in waypoint mode
	}

	TacticalOrder tacOrder;
	bool bCapture =  target->isCaptureable(Team::home->getId());
	bool bFireFromCurrentPos = controlGui.getCurrentRange() == FIRERANGE_CURRENT ? 1 : 0;
	if ( controlGui.getFireFromCurrentPos() )
		bFireFromCurrentPos = true;

	if (!bCapture)
	{
		if (!Team::home->isFriendly( target->getTeam() ))
		{
			tacOrder.init(ORDER_ORIGIN_PLAYER, TACTICAL_ORDER_ATTACK_OBJECT );
			tacOrder.targetWID = target->getWatchID();
			tacOrder.attackParams.type = bEnergyWeapons ? ATTACK_CONSERVING_AMMO : ATTACK_TO_DESTROY;
			tacOrder.attackParams.method = ATTACKMETHOD_RANGED;
			tacOrder.attackParams.range = FIRERANGE_OPTIMAL;
			tacOrder.attackParams.pursue = !bFireFromCurrentPos;
			tacOrder.moveParams.wayPath.mode[0] = controlGui.getWalk() ?  TRAVEL_MODE_SLOW : TRAVEL_MODE_FAST;

			target->getAppearance()->flashBuilding(1.3f,0.2f,0xffff0000);
		}
		else
		{
			//Do NOT Attack friendlies unless force Fire.
			// That TacOrder is issued elsewhere!!
			return;
		}
	}
	else
	{
		for (long i = 0; i < Team::home->getRosterSize(); i++)
		{
			Mover* pMover = (Mover*)Team::home->getMover( i );
			if ( pMover->isSelected() && pMover->getCommander()->getId() == Commander::home->getId() )
			{
				if ((pMover->getObjectClass() == BATTLEMECH) && 
					(pMover->getMoveType() == MOVETYPE_GROUND))
				{
					if ( target->getCaptureBlocker(pMover) )
					{
						g_soundSystem->playDigitalSample( INVALID_GUI );
						return;
					}
				}
				else
				{
					//Can't capture player has at least one support thing selected
					// At least until design tells me what they REALLY want!
					// -fs
					g_soundSystem->playDigitalSample( INVALID_GUI );
					return;
				}
			}
		}
		
		tacOrder.init(ORDER_ORIGIN_PLAYER, TACTICAL_ORDER_CAPTURE );
		tacOrder.targetWID = target->getWatchID();
		tacOrder.attackParams.type = ATTACK_NONE;
		tacOrder.attackParams.method = ATTACKMETHOD_RAMMING;
		tacOrder.attackParams.pursue = true;
		tacOrder.moveParams.wayPath.mode[0] = controlGui.getWalk() ?  TRAVEL_MODE_SLOW : TRAVEL_MODE_FAST;
	}
	
	g_soundSystem->playDigitalSample(BUTTON5);

	Team* pTeam = Team::home;
	
	for (long i = 0; i < pTeam->getRosterSize(); i++)
	{
		Mover* pMover = (Mover*)pTeam->getMover( i );
		if ( pMover->isSelected() && pMover->getCommander()->getId() == Commander::home->getId())
		{
			tacOrder.attackParams.range = (FireRangeType)pMover->attackRange;
			if (pMover->attackRange == FIRERANGE_CURRENT)
				tacOrder.attackParams.pursue = false;
			else if (controlGui.getFireFromCurrentPos())
				tacOrder.attackParams.pursue = false;
			else
				tacOrder.attackParams.pursue = true;
			tacOrder.pack(NULL, NULL);

			//---------------------------------------------------------------------
			// Helper function--perhaps this should just be a part of the mover and
			// group handleTacticalOrder() functions?
			if (MPlayer && !MPlayer->isServer())
				MPlayer->sendPlayerOrder(&tacOrder, false, 1, &pMover);
			else
				pMover->handleTacticalOrder(tacOrder);
		}
	}

	controlGui.setFireFromCurrentPos( 0 );
}

int MissionInterfaceManager::attackShort()
{
	controlGui.setRange( FIRERANGE_SHORT );
	if ( commandClicked && target )
	{
		doAttack();
		return 1;
	}
	return 0;
}

int MissionInterfaceManager::attackMedium()
{
	controlGui.setRange( FIRERANGE_MEDIUM );
	if ( commandClicked && target )
	{
		doAttack();
		return 1;
	}
	return 0;
}

int MissionInterfaceManager::attackLong()
{
	controlGui.setRange( FIRERANGE_LONG );
	if ( commandClicked && target )
	{
		doAttack();
		return 1;
	}
	return 0;
}

int MissionInterfaceManager::alphaStrike()
{
	return 0;
}
int MissionInterfaceManager::defaultAttack()
{
	
	controlGui.setRange( FIRERANGE_OPTIMAL );

	return 0;
}

int  MissionInterfaceManager::jump()
{
	if ( !canJump() )
	{
		g_soundSystem->playDigitalSample( INVALID_GUI );
		return 0;
	}
	
	if ( !controlGui.getJump() )
		controlGui.toggleJump();
	return 0;
}

int MissionInterfaceManager::stopJump()
{
	if ( controlGui.getJump() )
		controlGui.toggleJump();
	return 1;
}
void MissionInterfaceManager::doJump()
{
	if ( !target )
	{
		if (Terrain::IsGameSelectTerrainPosition(wPos))
		{
			long cellR, cellC;
			land->worldToCell(wPos, cellR, cellC);
		}
			
		bool canJump = canJumpToWPos();	
		if (canJump)
		{
			TacticalOrder tacOrder;
			tacOrder.init(ORDER_ORIGIN_PLAYER, TACTICAL_ORDER_JUMPTO_POINT, false);
		
			tacOrder.moveParams.wait = false;
			tacOrder.moveParams.wayPath.mode[0] = TRAVEL_MODE_JUMP;
			tacOrder.setWayPoint( 0, wPos );
			tacOrder.pack(NULL, NULL);

			handleOrders(tacOrder);

			controlGui.setDefaultSpeed();
			
			g_soundSystem->playDigitalSample(BUTTON5);
			controlGui.setDefaultSpeed();
		}
		else
		{
			userInput->setMouseCursor(mState_DONT);
			g_soundSystem->playDigitalSample( INVALID_GUI );
		}
	}
	else
	{
		userInput->setMouseCursor(mState_DONT);
		g_soundSystem->playDigitalSample( INVALID_GUI );
	}
}
		
int MissionInterfaceManager::fireFromCurrentPos()
{
	if ( !controlGui.getFireFromCurrentPos() )
		controlGui.toggleFireFromCurrentPos( );

	return forceShot();
}
int MissionInterfaceManager::stopFireFromCurrentPos()
{
	if ( controlGui.getFireFromCurrentPos() )
		controlGui.toggleFireFromCurrentPos();
	return 1;
}

void MissionInterfaceManager::doGuard(GameObject* who)
{
	if ( who )
	{
		TacticalOrder tacOrder;
		tacOrder.init(ORDER_ORIGIN_PLAYER, TACTICAL_ORDER_GUARD);
		tacOrder.targetWID = who->getWatchID();
		tacOrder.pack(NULL, NULL);	
		handleOrders( tacOrder );

		controlGui.setDefaultSpeed();
	}
	else
	{
		LocationNode path;
		path.location = wPos;
		path.run = true;
		path.next = NULL;

		TacticalOrder tacOrder;
		tacOrder.init(ORDER_ORIGIN_PLAYER, TACTICAL_ORDER_GUARD);
		tacOrder.initWayPath( &path );
		tacOrder.moveParams.wayPath.mode[0] = controlGui.getWalk() ?  TRAVEL_MODE_SLOW : TRAVEL_MODE_FAST;
		tacOrder.pack(NULL, NULL);	
		handleOrders( tacOrder );

		controlGui.setDefaultSpeed();
	}

	g_soundSystem->playDigitalSample(BUTTON5);
}
int MissionInterfaceManager::guard()
{
	if ( !controlGui.getGuard() )
		controlGui.toggleGuard();
	return 0;
}
int MissionInterfaceManager::stopGuard()
{
	if ( controlGui.getGuard() )
		controlGui.toggleGuard();
	return 0;
}
int MissionInterfaceManager::selectVisible()
{
	Team* pTeam = Team::home;
	for (long i=0;i<pTeam->getRosterSize();i++)
	{
		Mover* pMover = (Mover*)pTeam->getMover( i );
		if (pMover->getCommander()->getId() == Commander::home->getId())
		{
			pMover->setSelected( false );
		}
	}
		
	return addVisibleToSelection();
}

int MissionInterfaceManager::addVisibleToSelection()
{
	//----------------------------------------------------------------
	// Grab each member of the homeTeam and see if they are onScreen
	dragStart.Zero();		
	dragEnd = eye->getScreenRes();
	
	for (int i=0;i<ObjectManager->getNumMovers();i++)
	{
		if (ObjectManager->moverInRect(i,dragStart,dragEnd))
		{
			GameObjectPtr	mover = ObjectManager->getMover(i);
			if (mover && (mover->getCommanderId() == Commander::home->getId()) && !mover->isDisabled())
			{
				mover->setSelected( true );
			}
		}
	}

	dragStart.Zero();
	dragEnd.Zero();

	return 1;
}
int MissionInterfaceManager::aimLeg()
{
	if ( target )
	{
		BattleMech* pMech = dynamic_cast<BattleMech*>(target );
		
		if ( pMech && pMech->getMoveType() == MOVETYPE_GROUND)
		{
			if (!anySelectedWithoutAreaEffect())
			{
				//Can't aim shot.  No weapon which is not area effect available!
				userInput->setMouseCursor( mState_DONT );
				return 1;		
			}
			else if ( pMech->body[MECH_BODY_LOCATION_LLEG].damageState == IS_DAMAGE_DESTROYED 
				&& pMech->body[MECH_BODY_LOCATION_RLEG].damageState == IS_DAMAGE_DESTROYED )
			{
				userInput->setMouseCursor( mState_DONT );
				return 1;		
			}
			else  if ( !commandClicked )// cursor already set above, however if in doubt 
				return 1;
			else
			{
				TacticalOrder tacOrder;
				tacOrder.init(ORDER_ORIGIN_PLAYER, TACTICAL_ORDER_ATTACK_OBJECT);
				tacOrder.targetWID = target->getWatchID();
				tacOrder.attackParams.type = ATTACK_TO_DESTROY;
				tacOrder.attackParams.method = ATTACKMETHOD_RANGED;
				tacOrder.attackParams.range = FIRERANGE_OPTIMAL;
				tacOrder.attackParams.pursue = controlGui.getFireFromCurrentPos() ? false : true;
				tacOrder.moveParams.wayPath.mode[0] = controlGui.getWalk() ?  TRAVEL_MODE_SLOW : TRAVEL_MODE_FAST;
				
				bool bRight = pMech->body[MECH_BODY_LOCATION_LLEG].damageState == IS_DAMAGE_DESTROYED;
				tacOrder.attackParams.aimLocation = bRight ? MECH_BODY_LOCATION_RLEG :  MECH_BODY_LOCATION_LLEG;
				tacOrder.pack( NULL, NULL);
				handleOrders( tacOrder );

				g_soundSystem->playDigitalSample(BUTTON5);

				bAimedShot = true;
				return 1;
			}
		}
	}
	else
	{
		userInput->setMouseCursor( mState_DONT );
		return 1;
	}

	return 0;
}

int MissionInterfaceManager::aimArm()
{
	BattleMech* pMech = dynamic_cast<BattleMech*>(target);
		
	if ( pMech && pMech->getMoveType() == MOVETYPE_GROUND)
	{
		if ( anySelectedWithoutAreaEffect() && commandClicked )
		{
			TacticalOrder tacOrder;
			tacOrder.init(ORDER_ORIGIN_PLAYER, TACTICAL_ORDER_ATTACK_OBJECT);
			tacOrder.targetWID = target->getWatchID();
			tacOrder.attackParams.type = ATTACK_TO_DESTROY;
			tacOrder.attackParams.method = ATTACKMETHOD_RANGED;
			tacOrder.attackParams.pursue = controlGui.getFireFromCurrentPos() ? false : true;
			tacOrder.attackParams.range = FIRERANGE_OPTIMAL;
			tacOrder.moveParams.wayPath.mode[0] = controlGui.getWalk() ?  TRAVEL_MODE_SLOW : TRAVEL_MODE_FAST;
			bool bRight = pMech->body[MECH_BODY_LOCATION_LARM].damageState == IS_DAMAGE_DESTROYED;
			tacOrder.attackParams.aimLocation = bRight ? MECH_BODY_LOCATION_RARM :  MECH_BODY_LOCATION_LARM;
			tacOrder.pack( NULL, NULL);
			handleOrders( tacOrder );

			g_soundSystem->playDigitalSample(BUTTON5);

			bAimedShot = true;
			return 1;
		}

		if (!anySelectedWithoutAreaEffect())
		{
			//Can't aim shot.  No weapon which is not area effect available!
			userInput->setMouseCursor( mState_DONT );
			return 1;		
		}
		else if ( pMech->body[MECH_BODY_LOCATION_LARM].damageState == IS_DAMAGE_DESTROYED 
			&& pMech->body[MECH_BODY_LOCATION_RARM].damageState == IS_DAMAGE_DESTROYED )
		{
			userInput->setMouseCursor( mState_DONT );
			return 1;		
		}
		else // cursor already set above, however if in doubt 
			return 1;
	}
	else
	{
		userInput->setMouseCursor( mState_DONT );
		return 1;
	}
}

int MissionInterfaceManager::aimHead()
{
	if ( target )
	{
		BattleMech* pMech = dynamic_cast<BattleMech*>(target);
		
		if ( pMech && pMech->getMoveType() == MOVETYPE_GROUND)
		{
			if (anySelectedWithoutAreaEffect() && commandClicked )
			{
				TacticalOrder tacOrder;

				tacOrder.init(ORDER_ORIGIN_PLAYER, TACTICAL_ORDER_ATTACK_OBJECT);
				tacOrder.targetWID = target->getWatchID();
				tacOrder.attackParams.type = ATTACK_TO_DESTROY;
				tacOrder.attackParams.method = ATTACKMETHOD_RANGED;
				tacOrder.attackParams.pursue = controlGui.getFireFromCurrentPos() ? false : true;
				tacOrder.attackParams.range = FIRERANGE_OPTIMAL;
				tacOrder.moveParams.wayPath.mode[0] = controlGui.getWalk() ?  TRAVEL_MODE_SLOW : TRAVEL_MODE_FAST;
				tacOrder.attackParams.aimLocation = MECH_BODY_LOCATION_HEAD;
				tacOrder.pack( NULL, NULL);
				handleOrders( tacOrder );

				bAimedShot = true;
				g_soundSystem->playDigitalSample(BUTTON5);
			}
			else if (!anySelectedWithoutAreaEffect())
			{
				//Can't aim shot.  No weapon which is not area effect available!
				userInput->setMouseCursor( mState_DONT );
				return 1;		
			}
			else if ( pMech->body[MECH_BODY_LOCATION_HEAD].damageState == IS_DAMAGE_DESTROYED )
			{
				userInput->setMouseCursor( mState_DONT );
				return 1;		
			}
			else // cursor already set above, however if in doubt 
				return 1;
		}
		else
		{
			userInput->setMouseCursor( mState_DONT );
			return 1;
		}
	}
	else
	{
		userInput->setMouseCursor( mState_DONT );
		return 1;
	}

	return 0;
}

int MissionInterfaceManager::removeCommand()
{
	controlGui.doStop();
	return 1;
}
int MissionInterfaceManager::powerUp()
{
	TacticalOrder tacOrder;
	tacOrder.init(ORDER_ORIGIN_PLAYER, TACTICAL_ORDER_POWERUP, true);
	tacOrder.pack(NULL, NULL);					
	handleOrders( tacOrder );
	return 1;
}
int MissionInterfaceManager::powerDown()
{
	TacticalOrder tacOrder;
	tacOrder.init(ORDER_ORIGIN_PLAYER, TACTICAL_ORDER_POWERDOWN, true);
	tacOrder.pack(NULL, NULL);
	handleOrders( tacOrder );
	return 1;
}
int MissionInterfaceManager::changeSpeed()
{
 	if ( controlGui.isDefaultSpeed() )
		controlGui.toggleDefaultSpeed();
	return 0;
}

int MissionInterfaceManager::stopChangeSpeed()
{
	if ( !controlGui.isDefaultSpeed() )
		controlGui.toggleDefaultSpeed();
	return 1;
}
int MissionInterfaceManager::eject()
{
	if ( target && target->isMover() && target->getCommanderId() == Commander::home->getId() && !invulnerableON)
	{
		if (  commandClicked )
		{
			doEject( target );
		}

		userInput->setMouseCursor( mState_EJECT );
	}
	else if ( !controlGui.forceGroupBar.inRegion( userInput->getMouseX(), userInput->getMouseY() ) )
		userInput->setMouseCursor( mState_XEJECT );
	return 1;
}

int MissionInterfaceManager::bigAirStrike()
{
	Stuff::Vector3D v = makeAirStrikeTarget( wPos );

	IfaceCallStrike (ARTILLERY_LARGE,&v,NULL);
//	if ( !isPaused() )
		g_soundSystem->playSupportSample(SUPPORT_AIRSTRIKE);
	return 1;
}
int MissionInterfaceManager::smlAirStrike()
{
	Stuff::Vector3D v = makeAirStrikeTarget( wPos );

	IfaceCallStrike (ARTILLERY_SMALL,&v,NULL);
//	if ( !isPaused() )
		g_soundSystem->playSupportSample(SUPPORT_AIRSTRIKE);
	return 1;
}
int MissionInterfaceManager::snsAirStrike()
{
	Stuff::Vector3D v = makeAirStrikeTarget( wPos );

	IfaceCallStrike (ARTILLERY_SENSOR,&v,NULL);
//	if ( !isPaused() )
		g_soundSystem->playSupportSample(SUPPORT_PROBE);
	return 1;
}

Stuff::Vector3D MissionInterfaceManager::makeAirStrikeTarget( const Stuff::Vector3D& pos )
{
	Stuff::Vector3D newPos = pos;
/*	bool 	lineOfSight = Team::home->teamLineOfSight(pos);
	if ( !lineOfSight )
	{
		// need to offset x, y, random amounts up to five tiles... 
		long randX = rand();
		long randY = rand();
		long offsetX = randX % (128*5);
		long offsetY = randY % (128*5);
		if ( randX % 2 )
			offsetX = -offsetX;
		if ( randY % 2 )
			offsetY = -offsetY;

		newPos.x += offsetX;
		newPos.y += offsetY;
	}*/

	return newPos;
}

int MissionInterfaceManager::cameraAssign0()
{
	Camera::cameraZoom[0] = eye->cameraAltitude;
	Camera::cameraTilt[0] = eye->getProjectionAngle();

	return 1;
}

int MissionInterfaceManager::cameraAssign1()
{
	Camera::cameraZoom[1] = eye->cameraAltitude;
	Camera::cameraTilt[1] = eye->getProjectionAngle();

	return 1;
}

int MissionInterfaceManager::cameraAssign2()
{
	Camera::cameraZoom[2] = eye->cameraAltitude;
	Camera::cameraTilt[2] = eye->getProjectionAngle();

	return 1;
}

int MissionInterfaceManager::cameraAssign3()
{
	Camera::cameraZoom[3] = eye->cameraAltitude;
	Camera::cameraTilt[3] = eye->getProjectionAngle();

	return 1;
}

int MissionInterfaceManager::cameraNormal()
{
	if (eye)
		eye->allNormal();
		
	return 1;
}
int MissionInterfaceManager::cameraDefault()
{
	if (eye)
		eye->setCameraView(0);
 		
	return 1;
}
int MissionInterfaceManager::cameraMaxIn()
{
	if (eye)
		eye->setCameraView(1);
 		
	return 1;
}
int MissionInterfaceManager::cameraTight()
{
	if (eye)
		eye->setCameraView(2);
		
 	return 1;
}
int MissionInterfaceManager::cameraFour()
{
	if (eye)
		eye->setCameraView(3);
		
	return 1;
}

bool MissionInterfaceManager::anySelectedWithoutAreaEffect (void)
{
	//Scan the list of selected movers and see if any have at least one non-area effect weapon
	// Used for Called shots which cannot be called if weapon is area effect!!
	Team* pTeam = Team::home;
	for (long i = 0; i < pTeam->getRosterSize(); i++)
	{
		Mover* pMover = (Mover*)pTeam->getMover( i );
		if ( pMover->isSelected() && pMover->hasNonAreaWeapon() && pMover->getCommander()->getId() == Commander::home->getId())
		   	return true;
	}

	return false;
}

int MissionInterfaceManager::handleOrders( TacticalOrder& order)
{
	Team* pTeam = Team::home;

	//--------------------------------------------------------
	// First, find out how many are jumping and calc a list of
	// jump locations...
	bool isMoveOrder = false;
	Stuff::Vector3D moveGoals[MAX_MOVERS];
	long numMovers = 0;
	if (order.code == TACTICAL_ORDER_JUMPTO_POINT) {
		for (long i = 0; i < pTeam->getRosterSize(); i++) {
			Mover* pMover = pTeam->getMover(i);
			if (pMover->isSelected() && pMover->getCommander()->getId() == Commander::home->getId())
				numMovers++;
		}
		MoverGroup::calcJumpGoals(order.getWayPoint(0), numMovers, moveGoals, NULL);
		isMoveOrder = true;
		}
	else if (order.code == TACTICAL_ORDER_MOVETO_POINT) {
		for (long i = 0; i < pTeam->getRosterSize(); i++) {
			Mover* pMover = pTeam->getMover(i);
			if (pMover->isSelected() && pMover->getCommander()->getId() == Commander::home->getId())
				numMovers++;
		}
		MoverGroup::calcMoveGoals(order.getWayPoint(0), numMovers, moveGoals);
		isMoveOrder = true;
	}

	numMovers = 0;
	for (long i = 0; i < pTeam->getRosterSize(); i++)
	{
		Mover* pMover = (Mover*)pTeam->getMover( i );
		if ( pMover->isSelected() && pMover->getCommander()->getId() == Commander::home->getId() )
		{
			//---------------------------------------------------------------------
			// Helper function--perhaps this should just be a part of the mover and
			// group handleTacticalOrder() functions?
			if (isMoveOrder ) {
				order.setWayPoint(0, moveGoals[numMovers++]);
				order.pack(NULL, NULL);
			}
			if (MPlayer && !MPlayer->isServer()) {
				MPlayer->sendPlayerOrder(&order, false, 1, &pMover, 0, NULL, userInput->getKeyDown(s_waypointKey));
				//pMover->getPilot()->setExecutingQueue(false);
				//pMover->getPilot()->addQueuedTacOrder(order);					
				}
			else
			{
				order.attackParams.range = (FireRangeType)pMover->attackRange;
				order.pack( NULL, NULL );
				if ( userInput->getKeyDown( s_waypointKey ) ) // way point one
				{
					pMover->getPilot()->setExecutingQueue(FALSE);
					pMover->getPilot()->addQueuedTacOrder(order);					
				}
				else if ( controlGui.getMines() )
				{
					pMover->getPilot()->addQueuedTacOrder(order);
					pMover->getPilot()->setExecutingQueue(TRUE);
				}
				else
					pMover->handleTacticalOrder(order);
			}
		}
	}

	return 1;
}

int MissionInterfaceManager::scrollUp()
{
	float frameFactor = g_frameTime / baseFrameLength;
	float scrollFactor = scrollInc / eye->getScaleFactor() * frameFactor;
	eye->moveUp(scrollFactor);
	return 1;
}

int MissionInterfaceManager::scrollDown()
{
	float frameFactor = g_frameTime / baseFrameLength;
	float scrollFactor = scrollInc / eye->getScaleFactor() * frameFactor;
	eye->moveDown(scrollFactor);
	return 1;
}
int MissionInterfaceManager::scrollLeft()
{
	float frameFactor = g_frameTime / baseFrameLength;
	float scrollFactor = scrollInc / eye->getScaleFactor() * frameFactor;
	eye->moveLeft(scrollFactor);
	return 1;
}
int MissionInterfaceManager::scrollRight()
{
	float frameFactor = g_frameTime / baseFrameLength;
	float scrollFactor = scrollInc / eye->getScaleFactor() * frameFactor;
	eye->moveRight(scrollFactor);
	return 1;
}
int MissionInterfaceManager::zoomOut()
{
	float frameFactor = g_frameTime / baseFrameLength;
	eye->ZoomOut(zoomInc * frameFactor * eye->getScaleFactor());
	return 1;
}
int MissionInterfaceManager::zoomIn()
{
	float frameFactor = g_frameTime / baseFrameLength;
	eye->ZoomIn(zoomInc * frameFactor * eye->getScaleFactor());
	return 1;
}
int MissionInterfaceManager::zoomChoiceOut()
{
	float frameFactor = g_frameTime / baseFrameLength;
	eye->ZoomOut(zoomInc * frameFactor * 5.0f * eye->getScaleFactor());

	return 1;
}
int MissionInterfaceManager::zoomChoiceIn()
{
	float frameFactor = g_frameTime / baseFrameLength;
	eye->ZoomIn(zoomInc * frameFactor * 5.0f * eye->getScaleFactor());
 	return 1;
}
int MissionInterfaceManager::rotateLeft()
{
	float frameFactor = g_frameTime / baseFrameLength;
	realRotation += degPerSecRot * frameFactor;
	if (realRotation >= rotationInc)
	{
		eye->rotateLeft(rotationInc);
		realRotation = 0;
	}

	return 1;
}
int MissionInterfaceManager::rotateRight()
{
	float frameFactor = g_frameTime / baseFrameLength;
	realRotation -= degPerSecRot * frameFactor;
	if (realRotation <= -rotationInc)
	{
		eye->rotateRight(rotationInc);
		realRotation = 0;
	}

	return 1;
}
int MissionInterfaceManager::tiltUp()
{
	float frameFactor = g_frameTime / baseFrameLength;
	float scrollFactor = scrollInc / eye->getScaleFactor() * frameFactor;
	eye->tiltDown(scrollFactor * frameFactor * 30.0f);
	return 1;
}
int MissionInterfaceManager::tiltDown()
{
	float frameFactor = g_frameTime / baseFrameLength;
	float scrollFactor = scrollInc / eye->getScaleFactor() * frameFactor;
	eye->tiltUp(scrollFactor * frameFactor * 30.0f);
	return 1;
}
int MissionInterfaceManager::centerCamera()
{
	return 0;
}
int MissionInterfaceManager::rotateLightLeft()
{
	#ifndef FINAL
	eye->rotateLightLeft(rotationInc);
	#endif
	return 1;
}
int MissionInterfaceManager::rotateLightRight()
{
	#ifndef FINAL
	eye->rotateLightRight(rotationInc);
	#endif
	return 1;
}
int MissionInterfaceManager::rotateLightUp()
{
	#ifndef FINAL
	eye->rotateLightUp(rotationInc);
	#endif
	return 1;
}
int MissionInterfaceManager::rotateLightDown()
{
	#ifndef FINAL
	eye->rotateLightDown(rotationInc);
	#endif
	return 1;
}

int MissionInterfaceManager::drawTerrain()
{
	#ifndef FINAL
	drawTerrainTiles ^= TRUE;
	#endif
	return 1;
}
int MissionInterfaceManager::drawOverlays()
{
	#ifndef FINAL
	drawTerrainOverlays ^= TRUE;
	#endif
	return 1;
}
int MissionInterfaceManager::drawBuildings()
{
	#ifndef FINAL
	renderObjects ^= TRUE;
	#endif
	return 1;
}
int MissionInterfaceManager::showGrid()
{
	#ifndef FINAL
	drawTerrainGrid = !drawTerrainGrid;
	#endif
	return 1;
}
int MissionInterfaceManager::recalcLights()
{
	#ifndef FINAL
	Terrain::recalcLight ^= TRUE;
	Terrain::recalcShadows = false;
	#endif
	return 1;
}
int MissionInterfaceManager::drawClouds()
{
	#ifndef FINAL
	useClouds ^= TRUE;
	#endif
	return 1;
}
int MissionInterfaceManager::drawFog()
{
	#ifndef FINAL
	useFog ^= TRUE;
	#endif
	return 1;
}
int MissionInterfaceManager::usePerspective()
{
	#ifndef FINAL
	eye->usePerspective ^= true;
	#endif
	return 1;
}
int MissionInterfaceManager::toggleGUI()
{
	#ifndef FINAL
	drawGUIOn ^= true;
	toggledGUI = drawGUIOn;
	#endif
	return 1;
}
int MissionInterfaceManager::drawTGLShapes()
{
	#ifndef FINAL
	renderTGLShapes ^= true;
	#endif
	return 1;
}
int MissionInterfaceManager::drawWaterEffects()
{
	#ifndef FINAL
	useTerrainDetailTexture ^= true;
	#endif
	return 1;
}
int MissionInterfaceManager::recalcWater()
{
	#ifndef FINAL
	Terrain::mapData->recalcWater();
	#endif
	return 1;
}
int MissionInterfaceManager::drawShadows()
{
	#ifndef FINAL
	useShadows ^= TRUE;
	#endif
	return 1;
}
int MissionInterfaceManager::showLOSGrid()
{
	#ifndef FINAL
	drawLOSGrid ^= true;
	#endif
	return 1;
}

void MissionInterfaceManager::init (FitIniFilePtr loader)
{
	long result = loader->seekBlock("CameraData");
	gosASSERT(result == NO_ERR);
	
	result = loader->readIdFloat("ScrollIncrement",scrollInc);
	gosASSERT(result == NO_ERR);
	
	result = loader->readIdFloat("RotationIncrement",rotationInc);
	gosASSERT(result == NO_ERR);
	
	result = loader->readIdFloat("ZoomIncrement",zoomInc);
	gosASSERT(result == NO_ERR);
		
	result = loader->readIdFloat("ScrollLeft",screenScrollLeft);
	gosASSERT(result == NO_ERR);
		
	result = loader->readIdFloat("ScrollRight",screenScrollRight);
	gosASSERT(result == NO_ERR);
	
	result = loader->readIdFloat("ScrollUp",screenScrollUp);
	gosASSERT(result == NO_ERR);
	
	result = loader->readIdFloat("ScrollDown",screenScrollDown);
	gosASSERT(result == NO_ERR);

	result = loader->readIdFloat("BaseFrameLength",baseFrameLength);
	gosASSERT(result == NO_ERR);
	
	result = loader->readIdFloat("RotationDegPerSec",degPerSecRot);
	gosASSERT(result == NO_ERR);
	
	float missionDragThreshold;
	result = loader->readIdFloat("MouseDragThreshold",missionDragThreshold);
	gosASSERT(result == NO_ERR);

	float newThreshold = missionDragThreshold / Environment.screenHeight;
	userInput->setMouseDragThreshold(newThreshold);
	
	float missionDblClkThreshold;
	result = loader->readIdFloat("MouseDoubleClickThreshold",missionDblClkThreshold);
	gosASSERT(result == NO_ERR);
	
	userInput->setMouseDoubleClickThreshold(missionDblClkThreshold);

	swapResolutions();
}	
	
//--------------------------------------------------------------------------------------
void MissionInterfaceManager::destroy (void)
{
	hotKeyFont.destroy();
	hotKeyHeaderFont.destroy();

	delete keyboardRef;
	keyboardRef = NULL;
}	

//--------------------
// TO TEST LIGHTING MODEL
bool rotateLightRt = false;
bool rotateLightLf = false;
bool rotateLightUp = false;
bool rotateLightDn = false;
		
//--------------------------------------------------------------------------------------

void HandlePlayerOrder (MoverPtr mover, TacticalOrderPtr tacOrder) {

	//---------------------------------------------------------------------
	// Helper function--perhaps this should just be a part of the mover and
	// group handleTacticalOrder() functions?
	if (MPlayer && !MPlayer->isServer())
		MPlayer->sendPlayerOrder(tacOrder, false, 1, &mover);
	else
		mover->handleTacticalOrder(*tacOrder);
}
						
//--------------------------------------------------------------------------------------
void MissionInterfaceManager::drawVTOL (void)
{
	for (long vtolNum = 0;vtolNum < MAX_TEAMS; vtolNum++)
	{
		if ( paintingVtol[vtolNum] && vTol[vtolNum] )
		{
			vTol[vtolNum]->render();
			vTol[vtolNum]->renderShadows();
			
			if (dustCloud[vtolNum] && dustCloud[vtolNum]->IsExecuted())
			{
				gosFX::Effect::DrawInfo drawInfo;
				drawInfo.m_clipper = theClipper;
	
				Stuff::LinearMatrix4D 	shapeOrigin;
				Stuff::LinearMatrix4D	localToWorld;
				Stuff::LinearMatrix4D	localResult;
						
				Stuff::Vector3D dustPos = vTol[vtolNum]->position;
				Stuff::Point3D wakePos;
				wakePos.x = -dustPos.x;
				wakePos.y = dustPos.z;
				wakePos.z = dustPos.y;
				
				shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
				shapeOrigin.BuildTranslation(wakePos);
						
				drawInfo.m_parentToWorld = &shapeOrigin;
				if (!MLRVertexLimitReached)
					dustCloud[vtolNum]->Draw(&drawInfo);
			}
			
			if (recoveryBeam[vtolNum] && recoveryBeam[vtolNum]->IsExecuted())
			{
				gosFX::Effect::DrawInfo drawInfo;
				drawInfo.m_clipper = theClipper;
	
				Stuff::LinearMatrix4D 	shapeOrigin;
				Stuff::LinearMatrix4D	localToWorld;
				Stuff::LinearMatrix4D	localResult;
						
				Stuff::Vector3D dustPos = vTol[vtolNum]->position;
				Stuff::Point3D wakePos;
				wakePos.x = -dustPos.x;
				wakePos.y = dustPos.z;
				wakePos.z = dustPos.y;
				
				shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
				shapeOrigin.BuildTranslation(wakePos);
						
				drawInfo.m_parentToWorld = &shapeOrigin;
				if (!MLRVertexLimitReached)
					recoveryBeam[vtolNum]->Draw(&drawInfo);
			}
		}
	}
}

//--------------------------------------------------------------------------------------
void MissionInterfaceManager::render (void)
{
	if (drawGUIOn)
	{
		//-------------------------
		// Draw the dragSelect Box
		if (isDragging)
		{
			gos_VERTEX vertices[5];
			DWORD color = SB_WHITE;
			
			gos_SetRenderState( gos_State_AlphaMode, gos_Alpha_AlphaInvAlpha );
		
			gos_SetRenderState( gos_State_Specular,FALSE );

			gos_SetRenderState( gos_State_AlphaTest, TRUE );
					
			gos_SetRenderState( gos_State_Texture, 0 );

			gos_SetRenderState( gos_State_Filter, gos_FilterNone );

			Stuff::Vector4D screenPos;
			eye->projectZ( dragStart, screenPos );

			vertices[0].x 		= screenPos.x;
			vertices[0].y 		= screenPos.y;
			vertices[0].z		= 0.0;
			vertices[0].rhw		= 0.5;
			vertices[0].u		= 0.0;
			vertices[0].v		= 0.0;
			vertices[0].argb	= color;
			
			vertices[1].x 		= dragEnd.x;
			vertices[1].y 		= screenPos.y;
			vertices[1].z		= 0.0;
			vertices[1].rhw		= 0.5;
			vertices[1].u		= 0.0;
			vertices[1].v		= 0.0;
			vertices[1].argb	= color;
			
			vertices[2].x 		= dragEnd.x;
			vertices[2].y 		= dragEnd.y;
			vertices[2].z		= 0.0;
			vertices[2].rhw		= 0.5;
			vertices[2].u		= 0.0;
			vertices[2].v		= 0.0;
			vertices[2].argb	= color;
			
			vertices[3].x 		= screenPos.x;
			vertices[3].y 		= dragEnd.y;
			vertices[3].z		= 0.0;
			vertices[3].rhw		= 0.5;
			vertices[3].u		= 0.0;
			vertices[3].v		= 0.0;
			vertices[3].argb	= color;
		
			vertices[4].x 		= screenPos.x;
			vertices[4].y 		= screenPos.y;
			vertices[4].z		= 0.0;
			vertices[4].rhw		= 0.5;
			vertices[4].u		= 0.0;
			vertices[4].v		= 0.0;
			vertices[4].argb	= color;
			
			gos_DrawQuads(vertices,4);
			
			vertices[0].argb	= SB_BLACK;
			vertices[1].argb	= SB_BLACK;
			vertices[2].argb	= SB_BLACK;
			vertices[3].argb	= SB_BLACK;
			vertices[4].argb	= SB_BLACK;
			
			gos_DrawLines(&vertices[0],2);
			gos_DrawLines(&vertices[1],2);
			gos_DrawLines(&vertices[2],2);
			gos_DrawLines(&vertices[3],2);
		}

		if ( userInput->getKeyDown( s_waypointKey ) || controlGui.getMines() )
		{
			Team* pTeam = Team::home;
			for (long i=0;i<pTeam->getRosterSize();i++)
			{
				Mover* pMover = (Mover*)pTeam->getMover( i );
				if ( pMover->isSelected()  && pMover->getCommander()->getId() == Commander::home->getId() )
				{
					pMover->drawWaypointPath();
				}
			}
		}

		if ( bDrawHotKeys )
		{
			drawHotKeys();
			return;
		}

		if (g_framesSinceMissionStart > 3 /*&& !controlGui.inRegion(userInput->getMouseX(), userInput->getMouseY())*/ )
		{
#ifdef DRAW_CURSOR_CROSSHAIRS
			Stuff::Vector4D cursorPos;
			eye->projectZ(wPos,cursorPos);

			DWORD color = SB_WHITE;
			
			gos_SetRenderState( gos_State_Texture, 0 );

			gos_SetRenderState( gos_State_Filter, gos_FilterNone );

			Stuff::Vector4D				ul, br, pos1, pos2;

			pos1.x 		= cursorPos.x-5;
			pos1.y 		= cursorPos.y;
			pos1.z		= 0.000001f;
			
			pos2.x 		= cursorPos.x+5;
			pos2.y 		= cursorPos.y;
			pos2.z		= 0.000001f;
			pos2.w = pos1.w = 1.0f;

			{
				LineElement newElement(pos1,pos2,color,NULL,-1);
				newElement.draw();
			}

 			pos1.x 		= cursorPos.x;
			pos1.y 		= cursorPos.y-5;
			pos1.z		= 0.000001f;
			
			pos2.x 		= cursorPos.x;
			pos2.y 		= cursorPos.y+5;
			pos2.z		= 0.000001f;
			pos2.w = pos1.w = 1.0f;

			{
				LineElement newElement(pos1,pos2,color,NULL,-1);
				newElement.draw();
			}
			
#endif
			if (eye->usePerspective)
			{
				//Depth cue the cursor.
				Stuff::Vector3D Distance;
				Stuff::Point3D objPosition;
				Stuff::Point3D eyePosition(eye->getCameraOrigin());
				objPosition.x = -wPos.x;
				objPosition.y = wPos.z;
				objPosition.z = wPos.y;
		
				Distance.Subtract(objPosition,eyePosition);
				float eyeDistance = Distance.GetApproximateLength();
				float scaleFactor = eyeDistance / (Camera::maxClipDistance * 2.0f);
				scaleFactor = 1.5f - scaleFactor;

				userInput->setMouseScale(scaleFactor);			
 			}
		}
	}

	controlGui.render( isPaused() && !isPausedWithoutMenu() );
}	

void MissionInterfaceManager::initTacMap( PacketFile* file, int packet )
{
	file->seekPacket(packet);
	int size = file->getPacketSize( );
	BYTE* mem = new BYTE[size];

	file->readPacket( packet, mem );

	controlGui.initTacMapBuildings( mem, size );
	delete[] mem;

	file->seekPacket(packet + 1);
	size = file->getPacketSize( );
	mem = new BYTE[size];

	file->readPacket( packet + 1, mem );

	controlGui.initTacMap( mem, size );
	delete[] mem;

	swapTime = 0.f;

	for ( int i = 0; i < MAX_ICONS; i++ )
	{
		oldTargets[i] = NULL;
	}

	target = NULL;
}

void MissionInterfaceManager::printDebugInfo()
{
	if (userInput->isLeftClick() || userInput->isRightClick()) {
		long row, col;
		land->worldToCell(wPos, row, col);
		char debugString[256];
		if (target)
			sprintf(debugString, "POS = %s(%c) %d,%d [%ld, %ld] (%.4f, %.4f, %.4f) OBJ = %s\n", terrainStr[GameMap->getTerrain(row, col)], GameMap->getPassable(row, col) ? 'T' : 'F', GlobalMoveMap[0]->calcArea(row, col), GlobalMoveMap[1]->calcArea(row, col), row, col, wPos.x, wPos.y, wPos.z, target->getName());
		else
			sprintf(debugString, "POS = %s(%c) %d,%d [%ld, %ld] (%.4f, %.4f, %.4f)\n", terrainStr[GameMap->getTerrain(row, col)], GameMap->getPassable(row, col) ? 'T' : 'F', GlobalMoveMap[0]->calcArea(row, col), GlobalMoveMap[1]->calcArea(row, col), row, col, wPos.x, wPos.y, wPos.z);
		DEBUGWINS_print(debugString);
	}
}

void MissionInterfaceManager::doMove(const Stuff::Vector3D& pos)
{
	/*if ( controlGui.getGuardTower()  )
	{
		doGuardTower();
		return;
	}*/
	if ( controlGui.isAddingAirstrike() &&  controlGui.isButtonPressed( ControlGui::SENSOR_PROBE ))
	{
		// tmp hack
		Stuff::Vector3D tmp = wPos;
		wPos = pos;
		addAirstrike( );
		wPos = tmp;

		return;
	}
	
	LocationNode path;
	path.location = pos;
	path.run = true;
	path.next = NULL;

	TacticalOrder tacOrder;
	if ( controlGui.getJump() )
	{
		doJump();
		return;
	}
	else
		tacOrder.init(ORDER_ORIGIN_PLAYER, TACTICAL_ORDER_MOVETO_POINT, false);
	tacOrder.initWayPath(&path);
	tacOrder.moveParams.wait = false;
	tacOrder.moveParams.wayPath.mode[0] =  controlGui.getWalk() ? TRAVEL_MODE_SLOW : TRAVEL_MODE_FAST;
	if ( controlGui.getMines() )
		tacOrder.moveParams.mode = MOVE_MODE_MINELAYING;
	tacOrder.pack(NULL, NULL);

	handleOrders(tacOrder);
	
	g_soundSystem->playDigitalSample(BUTTON5);

	controlGui.setDefaultSpeed();
}

bool MissionInterfaceManager::makePatrolPath()
{
	Team* pTeam = Team::home;
	for (long i=0;i<pTeam->getRosterSize();i++)
	{
		Mover* pMover = (Mover*)pTeam->getMover( i );
		if ( pMover->isSelected() && pMover->getPilot()->getNumTacOrdersQueued() && pMover->getCommander()->getId() == Commander::home->getId() )
		{
			if ( pMover->isCloseToFirstTacOrder( wPos ) )
			{
				userInput->setMouseCursor( mState_LINK );
				if ( commandClicked )
				{
					pMover->getPilot()->setTacOrderQueueLooping( true );
				}

				return 1;
			}
			else 
			{
				Stuff::Vector3D tmp;
				tmp.Subtract( wPos, pMover->getPosition() );
				if ( tmp.GetLength() < 25 ) // random amount
				{
					userInput->setMouseCursor( mState_LINK );
					if ( commandClicked )
					{
						doMove(wPos);
						pMover->getPilot()->setTacOrderQueueLooping( true );
					}
					return 1;
				}
			}
		}
	}

	return 0;
}

int MissionInterfaceManager::forceShot()
{
	bForcedShot = true;

	userInput->setMouseCursor( mState_GENERIC_ATTACK );
	if ( commandClicked )
	{
			TacticalOrder tacOrder;
			tacOrder.init(ORDER_ORIGIN_PLAYER,  target ? TACTICAL_ORDER_ATTACK_OBJECT : TACTICAL_ORDER_ATTACK_POINT);
			tacOrder.targetWID = target ? target->getWatchID() : 0;
			if ( !target )
				tacOrder.attackParams.targetPoint = wPos;
			tacOrder.attackParams.type = bEnergyWeapons ? ATTACK_CONSERVING_AMMO : ATTACK_TO_DESTROY;
			tacOrder.attackParams.method = ATTACKMETHOD_RANGED;
			tacOrder.attackParams.range = (FireRangeType)FIRERANGE_OPTIMAL;
			tacOrder.attackParams.pursue = controlGui.getFireFromCurrentPos() ? false : true;
			tacOrder.moveParams.wayPath.mode[0] = controlGui.getWalk() ?  TRAVEL_MODE_SLOW : TRAVEL_MODE_FAST;
			
			g_soundSystem->playDigitalSample(BUTTON5);

			if (target)
				target->getAppearance()->flashBuilding(1.3f,0.2f,0xffff0000);

			handleOrders( tacOrder );
			return 1;
	}

	return 0;
}

void MissionInterfaceManager::selectForceGroup( int forceGroup, bool deselect )
{
	Team* pTeam = Team::home;
	bool bAllSelected = deselect ? false : true;
	if ( deselect )
	{

		for (long i=0;i<pTeam->getRosterSize();i++)
		{
			Mover* pMover = (Mover*)pTeam->getMover( i );
			if (pMover && pMover->isOnGUI() && (pMover->getCommander()->getId() == Commander::home->getId()))
			{
				pMover->setSelected( 0 );
			}
		}					
	}
	else // see if the whole force group is already seleced
	{
		for (long i=0;i<pTeam->getRosterSize();i++)
		{
			Mover* pMover = (Mover*)pTeam->getMover( i );
			if (pMover && pMover->isInUnitGroup( forceGroup ) && !pMover->isDisabled() && !pMover->isSelected() && pMover->isOnGUI() && (pMover->getCommander()->getId() == Commander::home->getId()))
			{
				bAllSelected = false;
			}
		}
	}

	for (long i=0;i<pTeam->getRosterSize();i++)
	{
		Mover* pMover = (Mover*)pTeam->getMover( i );
		if (pMover && pMover->isInUnitGroup( forceGroup ) && !pMover->isDisabled() && pMover->isOnGUI() && (pMover->getCommander()->getId() == Commander::home->getId()))
			pMover->setSelected( !bAllSelected );
	}
}
void MissionInterfaceManager::makeForceGroup( int forceGroup )
{
	Team* pTeam = Team::home;

	for (long i=0;i<pTeam->getRosterSize();i++)
	{
		Mover* pMover = (Mover*)pTeam->getMover( i );
		if (pMover && (pMover->getCommander()->getId() == Commander::home->getId()))
		{
			if ( pMover->isInUnitGroup( forceGroup ) )
				pMover->removeFromUnitGroup( forceGroup );
		}
	}					

	for (i=0;i<pTeam->getRosterSize();i++)
	{
		Mover* pMover = (Mover*)pTeam->getMover( i );
		if (pMover && (pMover->getCommander()->getId() == Commander::home->getId()))
		{
			if (pMover->isSelected() )
				pMover->addToUnitGroup( forceGroup );
		}
	}					
}

bool MissionInterfaceManager::moveCameraAround( bool lineOfSight, bool passable, bool ctrl, bool bGui, 
											   long moverCount, long nonMoverCount )
{
	bool bRetVal = 0;
	bool middleClicked = (!userInput->isLeftDrag() && !userInput->isRightDrag() && userInput->isMiddleClick());

	if ( (g_userPreferences.leftRightMouseProfile && ((userInput->isLeftClick() && userInput->getKeyDown(KEY_T)) || userInput->isLeftDoubleClick()) && target) 
		|| (!g_userPreferences.leftRightMouseProfile && userInput->isRightClick() && !userInput->isRightDrag() && target) && !bGui)
	{
		if (eye)
			((GameCamera *)eye)->setTarget(target);
	}

	if (!cameraClicked )
	{
		if ( mouseX <= (screenScrollLeft))
			scrollLeft();
		
		if ((mouseX >= (Environment.screenWidth - screenScrollRight)) )
			scrollRight();
		
		if ( (mouseY <= (screenScrollUp) && mouseY >= -screenScrollUp ) )
			scrollUp();
		
		if ( (mouseY >= (Environment.screenHeight - screenScrollDown) ) )
			scrollDown();
	}

	if (attilaXAxis < -ATTILA_THRESHOLD)
	{
		float oldScrollInc = scrollInc;
		scrollInc *= fabs(attilaXAxis) * ATTILA_FACTOR;
		scrollLeft();
		scrollInc = oldScrollInc;
	}
	
	if (attilaXAxis > ATTILA_THRESHOLD)
	{
		float oldScrollInc = scrollInc;
		scrollInc *= fabs(attilaXAxis) * ATTILA_FACTOR;
		scrollRight();
		scrollInc = oldScrollInc;
	}
	
	if (attilaYAxis < -ATTILA_THRESHOLD)
	{
		float oldScrollInc = scrollInc;
		scrollInc *= fabs(attilaYAxis) * ATTILA_FACTOR;
		scrollUp();
		scrollInc = oldScrollInc;
	}
	
	if (attilaYAxis > ATTILA_THRESHOLD)
	{
		float oldScrollInc = scrollInc;
		scrollInc *= fabs(attilaYAxis) * ATTILA_FACTOR;
		scrollDown();
		scrollInc = oldScrollInc;
	}
 	
	if (attilaRZAxis < -ATTILA_THRESHOLD)
	{
		eye->rotateLeft(rotationInc * fabs(attilaRZAxis) * g_frameTime * ATTILA_ROTATION_FACTOR);
	}
	
	if (attilaRZAxis > ATTILA_THRESHOLD)
	{
		eye->rotateRight(rotationInc * fabs(attilaRZAxis) * g_frameTime * ATTILA_ROTATION_FACTOR);
	}
	
 	//------------------------------------------------
	// Right drag is camera rotation and tilt now.
	if (cameraClicked)
	{
		bRetVal = 1;
		long mouseXDelta = userInput->getMouseXDelta();
		float actualRot = rotationInc * 0.1f * abs(mouseXDelta);
		if (mouseXDelta > 0)
		{
			eye->rotateRight(actualRot);
			bRetVal = 1;	
		}
		else if (mouseXDelta < 0)
		{
			eye->rotateLeft(actualRot);
			bRetVal = 1;	
		}
		
		long mouseYDelta = userInput->getMouseYDelta();
		float actualTilt = rotationInc * 0.1f * abs(mouseYDelta);
		if (mouseYDelta > 0)
		{
			eye->tiltDown(actualTilt);
			bRetVal = 1;
		}
		else if (mouseYDelta < 0)
		{
			eye->tiltUp(actualTilt);
			bRetVal = 1;
		}
		userInput->setMouseCursor( mState_ROTATE_CAMERA );
		
		//--------------------------------------------------
		// Zoom Camera based on Mouse Wheel input.
		long mouseWheelDelta = userInput->getMouseWheelDelta();
		if (mouseWheelDelta)
		{
			//Mouse wheel just picks zooms now.
			//float actualZoom = zoomInc * abs(mouseWheelDelta) * 0.0001f * eye->getScaleFactor();
			if (mouseWheelDelta > 0)
			{
				zoomChoiceOut();
			}
			else
			{
				zoomChoiceIn();
			}
	
			if ( target )
				userInput->setMouseCursor( makeTargetCursor( lineOfSight, moverCount, nonMoverCount ) );
			else
				userInput->setMouseCursor( makeNoTargetCursor( passable, lineOfSight, ctrl, bGui, moverCount, nonMoverCount ) );
			bRetVal = 1;
		}
			
		return bRetVal;
	}
	
	//--------------------------------------------------
	// Zoom Camera based on Mouse Wheel input.
	long mouseWheelDelta = userInput->getMouseWheelDelta();
	if (mouseWheelDelta)
	{
		//Mouse wheel just picks zooms now.
		//float actualZoom = zoomInc * abs(mouseWheelDelta) * 0.0001f * eye->getScaleFactor();
		if (mouseWheelDelta > 0)
		{
			zoomChoiceOut();
		}
		else
		{
			zoomChoiceIn();
		}

		if ( target )
			userInput->setMouseCursor( makeTargetCursor( lineOfSight, moverCount, nonMoverCount ) );
		else
			userInput->setMouseCursor( makeNoTargetCursor( passable, lineOfSight, ctrl, bGui, moverCount, nonMoverCount ) );
		bRetVal = 1;
	}

	//-----------------------------------------------------------------
	// If middle mouse button is pressed, go to normal tilt, 50% zoom	
	if (middleClicked)
	{
		cameraNormal();
	}				
	
	doDrag(bGui);

	return bRetVal;
}

bool MissionInterfaceManager::canJump()
{
	bool canJump = true;
	long i=0;

	Team* pTeam = Team::home;
	
	while ((i<pTeam->getRosterSize()))
	{
		Mover* pMover = (Mover*)pTeam->getMover( i );
		if ( pMover->isSelected() && pMover->getCommander()->getId() == Commander::home->getId() )
		{
			if ( !pMover->canJump() )
			{
				canJump = false;
				break;
			}
		}
		i++;
	}

	return canJump;
}

bool MissionInterfaceManager::canJumpToWPos()
{
	bool passable = 0;
	if ( Terrain::IsGameSelectTerrainPosition( wPos ) )
	{
		long cellR, cellC;
		land->worldToCell(wPos, cellR, cellC);
		passable = GameMap->getPassable(cellR,cellC);
	}
	
	if (passable)
	{
		bool canJump = true;
		long i=0;

		Team* pTeam = Team::home;
		
		while ((i<pTeam->getRosterSize()))
		{
			Mover* pMover = (Mover*)pTeam->getMover( i );
			if ( pMover->isSelected() && pMover->getCommander()->getId() == Commander::home->getId() )
			{
				if ( pMover->canJump() )
				{
					TacticalOrder lastQueuedTacOrder;
					long err = pMover->getPilot()->peekQueuedTacOrder(-1, &lastQueuedTacOrder);
					Stuff::Vector3D lastWayPoint;
					if (err)
						lastWayPoint = pMover->getPosition();
					else
						lastWayPoint = lastQueuedTacOrder.getWayPoint(0);
					Stuff::Vector3D distance;
					distance.Subtract(lastWayPoint,wPos);

					//JUMP distance is always JUST horizontal.  NO Vertical!!
					distance.z = 0.0f;
					float dist = distance.GetLength();
					
					if (dist > pMover->getJumpRange())
						canJump = false;
				}
				else
					canJump = false;
			}
			i++;
		}

		return canJump;
	}

	return 0;
}

void MissionInterfaceManager::doDrag(bool bGui)
{
	//---------------------------------------------------------------------------
	// Check if we wanted to select all visible.  If so, do it!
//	if ( ((GetAsyncKeyState( VK_LBUTTON ))) && !bGui && !cameraClicked )
	if ( ((gos_GetKeyStatus( KEY_LMOUSE ) == KEY_PRESSED)) && !bGui && !cameraClicked
		&& !dragStart.x)
	{
	
 			dragStart.x = wPos.x;
			dragStart.y = wPos.y;
			dragStart.z = wPos.z;
			isDragging = 0;			
	}
	
	else if ((gos_GetKeyStatus( KEY_LMOUSE ) == KEY_HELD)
		&& !isDragging && ( !(dragStart.x == 0.f && dragStart.y == 0.f )
		&& (dragStart.x != wPos.x && dragStart.y != wPos.y ) ) )
	{
		isDragging = TRUE;
		dragEnd.x = mouseX;
		dragEnd.y = mouseY;
	}
	
	else if (isDragging)
	{
		dragEnd.x = mouseX;
		dragEnd.y = mouseY;
	
		if ( (gos_GetKeyStatus( KEY_LMOUSE ) == KEY_RELEASED
			|| gos_GetKeyStatus( KEY_LMOUSE ) == KEY_FREE) && isDragging)
		{
			//----------------------------------------------
			// We should select everything in the drag box.

			// AND deselect anything outside of it.

			GameObject*	objsInRect[MAX_ICONS];
			int count = 0;
			memset( objsInRect, 0, sizeof( GameObject* ) * 16 );
			for (long i=0;i<ObjectManager->getNumMovers();i++)
			{
				Stuff::Vector3D screenStart;
				Stuff::Vector4D screenPos;
				eye->projectZ( dragStart, screenPos );
				screenStart.x = screenPos.x;
				screenStart.y = screenPos.y;
			   GameObjectPtr	mover = ObjectManager->getMover(i);
			   if (ObjectManager->moverInRect(i,screenStart,dragEnd) && 
				   mover->getCommanderId() == Commander::home->getId() && 
				   count < MAX_ICONS
				   && !mover->isDisabled())
				{
				   objsInRect[count] = mover;
				   count++;
				}
			}

			if ( count )
			{
				// empty the info window of any previously selected guys
				controlGui.setInfoWndMover( NULL );

				if ( !userInput->shift() )
				{
					for (i=0;i<ObjectManager->getNumMovers();i++)
					{
						ObjectManager->getMover(i)->setSelected( 0 );
					}
				}

				for ( i = 0; i < count; i ++ )
				{
					objsInRect[i]->setSelected( true );
				}

				if ( count == 1 )
					(controlGui).setInfoWndMover( (Mover*)objsInRect[0] );
			}
			
			dragStart.Zero();
			dragEnd.Zero();		
			isDragging = FALSE;
		}
	}
	else if ( gos_GetKeyStatus(KEY_LMOUSE) == KEY_RELEASED )
	{
		dragStart.Zero();
	}
}

bool MissionInterfaceManager::canAddVehicle( const Stuff::Vector3D& pos )
{
	//We're probably going to use this alot!
	long commanderID = Commander::home->getId();

 	if ( paintingVtol[commanderID] )
		return 0;
	long tileI, tileJ;
	land->worldToTile( pos, tileJ, tileI );

	if ( tileJ > -1 && tileJ < land->realVerticesMapSide 
		 && tileI > -1 && tileI < land->realVerticesMapSide )
	{
		if ( Team::home->teamLineOfSight(pos, 0.0f) )
		{
			if ( GameMap->getPassable(pos) && !land->getWater(const_cast<Stuff::Vector3D&>(pos)) )
				return true;
			else 
				return false;
		}	
	}

	return false;
}

bool MissionInterfaceManager::canRecover( const Stuff::Vector3D& pos )
{
	//We're probably going to use this alot!
	long commanderID = Commander::home->getId();

 	if ( paintingVtol[commanderID] )
		return 0;
		
	long tileI, tileJ;
	land->worldToTile( pos, tileJ, tileI );

	if ( tileJ > -1 && tileJ < land->realVerticesMapSide 
		 && tileI > -1 && tileI < land->realVerticesMapSide )
	{
		if ( Team::home->teamLineOfSight(pos, 0.0f) )
		{
			if ( target && (target->getObjectClass() == BATTLEMECH) && target->isDisabled() && !target->isDestroyed() )
				return true;
			else 
				return false;
		}	
	}

	return false;
}

long MissionInterfaceManager::makeTargetCursor( bool lineOfSight, long moverCount, long nonMoverCount)
{
	//We're probably going to use this alot!
	long commanderID = Commander::home->getId();

 	long currentCursor = makeRangeCursor( lineOfSight );
	if ( !moverCount && !nonMoverCount )
		currentCursor = mState_NORMAL;

	if ( ( userInput->getKeyDown( s_waypointKey ) ) )
	{
		currentCursor = mState_DONT;
		return currentCursor;
	}

	if ( controlGui.isSelectingInfoObject()  )
	{
		if ( target->isMover() )
		{
			currentCursor = mState_INFO;
		}
		else 
			currentCursor = mState_DONT;
		
		return currentCursor;
	}
	
	else if ( controlGui.isAddingVehicle() && !paintingVtol[commanderID] )
	{
		currentCursor = mState_DONT;
		return currentCursor;
	}

	else if ( controlGui.isAddingSalvage() && !paintingVtol[commanderID] )
	{
		if ( canSalvage(target) )
		{
			currentCursor = mState_SALVAGE;
			target->setTargeted(true);
		}
		else
			currentCursor = mState_XSALVAGE;

		return currentCursor;
	}
	else if ( controlGui.isAddingAirstrike() && !paintingVtol[commanderID] )
	{
		if ( controlGui.getButton( ControlGui::SENSOR_PROBE )->state == ControlButton::PRESSED )
		{
			currentCursor = lineOfSight ? mState_SENSORSTRIKE : mState_UNCERTAIN_AIRSTRIKE ;
		}
		else
		{
			currentCursor = lineOfSight ? mState_AIRSTRIKE : mState_UNCERTAIN_AIRSTRIKE ;
		}
		return currentCursor;
	}
	else if ( controlGui.getSalvage()  )
	{
		if ( canSalvage( target ) )
		{
			currentCursor = mState_SALVAGE;
			target->setTargeted(true);
		}
		else
			currentCursor = mState_XSALVAGE;
		
		return currentCursor;
	}
	else if ( controlGui.getRepair() )
	{
		if ( canRepair( target ) )
			currentCursor = mState_REPAIR;
		else
			currentCursor = mState_XREPAIR;

		if (target)
			target->setTargeted(true);

		return currentCursor;
	}
	else if ( controlGui.getMines() )
	{
		currentCursor = mState_DONT;
		return currentCursor;
	}
	else if ( controlGui.getGuardTower() )
	{
		currentCursor = lineOfSight ? mState_GUARDTOWER_LOS : mState_GUARDTOWER;
		return currentCursor;
	}
	else if ( target->isCaptureable(Team::home->getId() ) && moverCount )
	{
		for ( int i = 0; i < Team::home->getRosterSize(); i++ )
		{
			Mover* pMvr = Team::home->getMover( i );
			if ( pMvr->isSelected() && pMvr->getCommander()->getId() == Commander::home->getId() )
			{
				if ((pMvr->getObjectClass() == BATTLEMECH) &&
					(pMvr->getMoveType() == MOVETYPE_GROUND))
				{
					if ( target->getCaptureBlocker( pMvr) )
					{
						currentCursor = mState_XCAPTURE;
						break;
					}
					else
					{
						currentCursor = mState_CAPTURE;
					}
				}
				else
				{
					currentCursor = mState_XCAPTURE;
				}
			}
		}
	}	

	switch (target->getObjectClass())
	{
		case BATTLEMECH:
		case GROUNDVEHICLE:
		case ELEMENTAL:
		{
			//---------------------------------------------------------------------------------------
			// if target is enemy battleMech, select it, change cursor and check below for leftClick
			if (target->getTeam() && !target->getTeam()->isFriendly(Team::home) && !target->getTeam()->isNeutral( Team::home ) )
			{
				target->setTargeted(true); 
				if ( target->isDisabled() )
				{
					currentCursor = mState_NORMAL;
	//				target = NULL;
				}
			}
			else
			{
				//-------------------------------------------------------------------
				// Otherwise we over a friendly.  Assume we intend to select them if
				// we click.  If we shift-click, just add them.
				target->setTargeted(true);		//For friendlies, let us know we need to draw their names!

				currentCursor = mState_NORMAL;
				controlGui.setRolloverHelpText( IDS_UNIT_SELECT_HELP );
			}
		}
		break;
			
		case BUILDING:
		case TREEBUILDING:			
		case TURRET:
		case GATE:
		case TREE:
		case TERRAINOBJECT:
		{
			if ( target->getObjectType()->getSubType() == BUILDING_SUBTYPE_LANDBRIDGE )
			{
				//Check if forcing fire or fire from current.  If so, shoot away.
				if ( gos_GetKeyStatus( (gosEnum_KeyIndex)s_commands[FORCE_FIRE_KEY].key ) == KEY_HELD || 
					 controlGui.getFireFromCurrentPos() )
				{
					target->setTargeted(true);
				}
				else
				{
					target = NULL;
					// find out if this position is passable.  Landbridges kinda are, or they'd be useless.
					long cellR, cellC;
					bool passable = 1;
					bool lineOfSight = 0;
					if ( Terrain::IsGameSelectTerrainPosition( wPos ) )
					{
						land->worldToCell(wPos, cellR, cellC);
						passable = GameMap->getPassable(cellR, cellC);
					}
	
					return makeNoTargetCursor( passable, lineOfSight, 0, 0, moverCount, nonMoverCount );
				}
			}
			//------------------------------------------------------------------
			// No matter what side this is on, change the cursor and select.
			target->setTargeted(true);
			TeamPtr targetTeam = target->getTeam();

			if ( targetTeam && Team::home->isFriendly( targetTeam ) && 
				target->getFlag(OBJECT_FLAG_CANREFIT) && target->getFlag(OBJECT_FLAG_MECHBAY) )  
			{
				if (canRepairBay(target))
				{
					currentCursor = mState_REPAIR;
				}
				else
				{
					currentCursor = mState_XREPAIR;
				}
			}
			else if ( targetTeam && !controlGui.getGuard() &&
				Team::home->isFriendly( targetTeam ) && 				
				gos_GetKeyStatus( (gosEnum_KeyIndex)s_commands[FORCE_FIRE_KEY].key ) != KEY_HELD // not forcing fire
				&& controlGui.getCurrentRange() == FIRERANGE_OPTIMAL ) // range attacks now are force fire
			{
				return makeNoTargetCursor( false, lineOfSight, 0, 0, moverCount, nonMoverCount );
			}
		}
		break;

		case BRIDGE:
			target = NULL;
			return makeNoTargetCursor( false, lineOfSight, 0, 0, moverCount, nonMoverCount );
			break;

		case DEBRIS:
		{
			//----------------------------------
			// Do nothing for Debris right now.
		}
		break;			
	}
	
	if ( controlGui.getGuard() )
	{
		currentCursor = mState_GUARD;
		if (target)
			target->setTargeted(true);
	}
	else if ( gos_GetKeyStatus( (gosEnum_KeyIndex)s_commands[FORCE_FIRE_KEY].key ) == KEY_HELD
		|| ( controlGui.getCurrentRange() != FIRERANGE_OPTIMAL && controlGui.getCurrentRange() != FIRERANGE_CURRENT ) )
	{
		currentCursor = makeRangeCursor( lineOfSight );
	}

	return currentCursor;
}

long MissionInterfaceManager::makeRangeCursor( bool lineOfSight)
{
	long currentCursor = mState_NORMAL;
	long currentRange = controlGui.getCurrentRange();

	if ( bEnergyWeapons )
		return lineOfSight ? mState_ENERGY_WEAPONS_LOS : mState_ENERGY_WEAPONS;
	
	switch( currentRange )
	{
	case FIRERANGE_SHORT:
		currentCursor = lineOfSight ? mState_SHRTRNG_LOS :  mState_SHRTRNG_ATTACK;
		break;
	case FIRERANGE_MEDIUM:
		currentCursor = lineOfSight ? mState_MEDRNG_LOS :   mState_MEDRNG_ATTACK;
		break;
	case FIRERANGE_LONG:
		currentCursor =  lineOfSight ? mState_LONGRNG_LOS :  mState_LONGRNG_ATTACK;
		break;
	case FIRERANGE_OPTIMAL:
		currentCursor =  lineOfSight ? mState_ATTACK_LOS :  mState_GENERIC_ATTACK;
		break;

	case FIRERANGE_CURRENT:
		currentCursor = lineOfSight ? mState_CURPOS_ATTACK_LOS : mState_CURPOS_ATTACK;
		break;
	}

	if ( controlGui.getFireFromCurrentPos() )
		currentCursor = lineOfSight ? mState_CURPOS_ATTACK_LOS : mState_CURPOS_ATTACK;

	return currentCursor;
}

long MissionInterfaceManager::makeNoTargetCursor( bool passable, bool lineOfSight, bool bCtrl, bool bGui,
												 long moverCount, long nonMoverCount )
{
	//-------------------------------------------------------------------------
	// We are not over enything.  Switch to move cursor based on input.
	// If we are on passable terrain, move.
	// If passable and runCommand, run.
	// If passable and jumpCommand and canJump, jump.
	// If not passable, dont.
	// If passable and jumpCommand and cantJump, dont.

	//We're probably going to use this alot!
	long commanderID = Commander::home->getId();
 	int cursorType = makeMoveCursor( lineOfSight );
	if ( !moverCount )
		cursorType = mState_NORMAL;

	if ( gos_GetKeyStatus( (gosEnum_KeyIndex)s_commands[FORCE_FIRE_KEY].key ) == KEY_HELD // not forcing fire
				|| controlGui.getFireFromCurrentPos() )
	{
		return makeRangeCursor( lineOfSight );
	}

	if ( !passable && !selectionIsHelicopters() )
		cursorType = mState_DONT;
		
	if ( controlGui.isSelectingInfoObject() )
	{
		cursorType = mState_DONT;
		return cursorType;
	}
	else if ( controlGui.isAddingAirstrike() && !paintingVtol[commanderID] )
	{
		if ( bGui && (!controlGui.isOverTacMap() || !controlGui.isButtonPressed( ControlGui::SENSOR_PROBE)) )
			cursorType = mState_NORMAL;
		else
		{
			if ( controlGui.getButton( ControlGui::SENSOR_PROBE )->state == ControlButton::PRESSED )
			{
				cursorType = lineOfSight ? mState_SENSORSTRIKE : mState_UNCERTAIN_AIRSTRIKE ;
			}
			else
				cursorType = lineOfSight ? mState_AIRSTRIKE : mState_UNCERTAIN_AIRSTRIKE;
		}
		return cursorType;
	}
	else if ( controlGui.isAddingVehicle() )
	{
		if ( canAddVehicle( wPos ) && !bGui)
			cursorType = mState_VEHICLE;
		else if ( !paintingVtol[commanderID] && ! bGui)
			cursorType = mState_CANTVEHICLE;
		else if ( bGui )
			cursorType = mState_NORMAL;
	}
	else if ( controlGui.isAddingSalvage())
	{
		if ( canRecover( wPos ) && !bGui )
			cursorType = mState_SALVAGE;
		else if ( !paintingVtol[commanderID] && !bGui) 
			cursorType = mState_XSALVAGE;
		else if ( bGui )
			cursorType = mState_NORMAL;
	}
	else if ( userInput->getKeyDown( s_waypointKey ) || controlGui.getMines() )
	{
		if ( controlGui.getMines() )
		{
			if ( !passable )
				cursorType = mState_XMINES;
			else
				cursorType = mState_LAYMINES;
		}

		else
		{
			cursorType  = lineOfSight ? mState_WALKWAYPT_LOS : mState_WALKWAYPT;
			if ( controlGui.getRun() )
				cursorType  = lineOfSight ? mState_RUNWAYPT_LOS : mState_RUNWAYPT;
			else if ( controlGui.getJump() )
				cursorType  = lineOfSight ? mState_JUMPWAYPT_LOS: mState_JUMPWAYPT;
		}

		return cursorType;
	}

	else if ( controlGui.getJump() )
	{
		if ( canJumpToWPos() )
		{	
			if ( bCtrl )
				cursorType = lineOfSight ? mState_JUMPWAYPT_LOS : mState_JUMPWAYPT;
			else
				cursorType = makeJumpCursor( lineOfSight );

			controlGui.setRolloverHelpText( IDS_UNIT_HELP );
		}
		else
			cursorType = mState_DONT;
			
		if ( !passable && !selectionIsHelicopters() )
			cursorType = mState_DONT;
	}
	else if ( controlGui.getRun() && moverCount )
	{
		if ( bCtrl )
			cursorType = lineOfSight ? mState_RUNWAYPT_LOS : mState_RUNWAYPT;
		else
			cursorType = makeRunCursor( lineOfSight );
			
		if ( !passable && !selectionIsHelicopters() )
			cursorType = mState_DONT;
		else
			controlGui.setRolloverHelpText( IDS_UNIT_HELP );
	}
	else if ( controlGui.getMines() )
	{
		cursorType = mState_LAYMINES;
		if ( !passable )
			cursorType = mState_XMINES;
	}
	else if ( controlGui.getSalvage() )
	{
		cursorType = makeMoveCursor( lineOfSight );
		if ( !passable )
			cursorType = mState_DONT;
	}
	else if ( controlGui.getRepair() )
	{
		cursorType = makeMoveCursor( lineOfSight );
		if ( !passable )
			cursorType = mState_DONT;
	}
	else if ( controlGui.getGuard() )
	{
		cursorType = lineOfSight ? mState_GUARD_LOS : mState_GUARD;
		if ( !passable )
			cursorType = mState_DONT;
	}
	else if ( controlGui.getGuardTower() )
	{
		cursorType = lineOfSight ? mState_GUARDTOWER_LOS : mState_GUARDTOWER;
	}
	else if ( moverCount )
	{
		controlGui.setRolloverHelpText( IDS_UNIT_HELP );
	}
	
	userInput->setMouseFrame(0);		

	return cursorType;
}

void MissionInterfaceManager::addAirstrike()
{
	if ( controlGui.isButtonPressed( ControlGui::LARGE_AIRSTRIKE ) )
		bigAirStrike();
	else if ( controlGui.isButtonPressed( ControlGui::LARGE_AIRSTRIKE ) )
		smlAirStrike();
	else if ( controlGui.isButtonPressed( ControlGui::SENSOR_PROBE ) )
		snsAirStrike();
	else
		gosASSERT( false );

	controlGui.unPressAllVehicleButtons();
}

//-----------------------------------------------------------------------------

MoverPtr BringInReinforcement (long vehicleID, long rosterIndex, long commanderID, Stuff::Vector3D pos, bool exists) {

	MoverInitData data;
	memset( &data, 0, sizeof( data ) );
	char* vehicleFile = (char*)MissionInterfaceManager::instance()->getSupportVehicleNameFromID(vehicleID);
	if (!vehicleFile) {
		char s[1024];
		sprintf(s, "BringInReinforcement: null behicleFile (%ld)", vehicleID);
		STOP((s));
		//return(NULL);
	}
	strcpy( data.pilotFileName, vehicleFile );
	strcpy( data.brainFileName, "pbrain" );
	data.controlType = CONTROL_AI;
	data.controlDataType = 2;
	data.rosterIndex = rosterIndex;
	data.objNumber = vehicleID;
	data.position = pos;
	data.rotation = 0;
	data.teamID = Commander::commanders[commanderID]->team->getId();
	data.commanderID = commanderID;
	data.active = 1;
	data.exists = exists;
	data.capturable = 0;
	data.baseColor = g_userPreferences.baseColor;
	data.highlightColor1 = g_userPreferences.highlightColor;
	data.highlightColor2 = g_userPreferences.highlightColor;

	if ( MPlayer )
	{
		MC2Player* pInfo = MPlayer->getPlayerInfo( commanderID );
		if ( pInfo )
		{
			data.baseColor = MPlayer->colors[pInfo->baseColor[BASECOLOR_TEAM]];
			data.highlightColor1 = MPlayer->colors[pInfo->stripeColor];
			data.highlightColor2 = MPlayer->colors[pInfo->stripeColor];
		}
	}

	long moverId = mission->addMover( &data );
	
	if ((moverId != -1) && (moverId != 0))
	{
		//Start the helicopters landed!!
		MoverPtr newMover = (MoverPtr)ObjectManager->get(moverId);
		if (!newMover)
			return NULL;

		if (newMover->getMoveType() == MOVETYPE_AIR)
			newMover->startShutDown();

		if (!MPlayer)
			newMover->setLocalMoverId(0);

		//Somehow this is set during a quickLoad?
		newMover->disableThisFrame = false;

		return(newMover);
	}

	return NULL;
}

//-----------------------------------------------------------------------------

void MissionInterfaceManager::addVehicle( const Stuff::Vector3D& pos )
{
	
//	LogisticsPilot* pPilot = LogisticsData::instance->getFirstAvailablePilot();

//	const char* pChar = pPilot->getFileName();
//	if ( !pChar )
//		return;

	// Check if vehicleFile is NULL.  If it is, simply get it.
	// Can happen if we get here BEFORE beginVtol?
	// Happens when we select a vehicle which costs exactly the number of points we have left.
	// Heidi deletes the points, which shuts off the button, which causes getVehicleName to return NULL.
	// D'OH!!!
	// Delete the points in getVehicleName when the vehicle is actually added!!!
	// -fs
	if (!vehicleFile)
		vehicleFile = controlGui.getVehicleName(vehicleID[Commander::home->getId()]);

	if ( !vehicleFile )
		return;

	reinforcement = BringInReinforcement(vehicleID[Commander::home->getId()], 255, Commander::home->getId(), pos, true);
}

void MissionInterfaceManager::beginVtol (long supportID, long commanderID, Stuff::Vector3D* reinforcePos, MoverPtr salvageTarget)
{
	Stuff::Vector3D vtolPos = wPos;
	if (reinforcePos)
		vtolPos = *reinforcePos;

	if (supportID == -1) {
		vehicleFile = controlGui.getVehicleName( vehicleID[commanderID] );
		}
	else 
	{
		//-----------------------
		// used in multiplayer...
		vehicleID[commanderID] = supportID;
		vehicleFile = controlGui.getVehicleNameFromID(supportID);
	}

	if (vehicleID[commanderID] == 0) {
		char s[1024];
		sprintf(s, "beginvtol: supportID = %ld, commanderID = %ld, vehicleID = %ld, vehicleFile = %s", supportID, commanderID, vehicleID[commanderID], vehicleFile);
		PAUSE((s));
	}

	if ( !vTol[commanderID] )
	{
		if (vehicleID[commanderID] != 147)		//NOT a recovery vehicle.  Standard VTOL
		{
			if (supportID == -1) 
			{
				//Do not call this unless the commanderID passed in equals us or will you do something outside here?
				if (MPlayer && !reinforcePos) 
				{
					vPos[commanderID] = vtolPos;
					MPlayer->sendReinforcement(vehicleID[commanderID], 255, "noone", commanderID, vPos[commanderID], 0);
					return;
				}
			}
			
			AppearanceType* appearanceType = appearanceTypeList->getAppearance( BLDG_TYPE << 24, "vtol" );
			vTol[commanderID] = new BldgAppearance;
			vTol[commanderID]->init( appearanceType );
			
			if (!dustCloud[commanderID] && g_userPreferences.useNonWeaponEffects)
			{
				if (strcmp(weaponEffects->GetEffectName(VTOL_DUST_CLOUD),"NONE") != 0)
				{
					//--------------------------------------------
					// Yes, load it on up.
					unsigned flags = 0;
		
					Check_Object(gosFX::EffectLibrary::Instance);
					gosFX::Effect::Specification* gosEffectSpec = gosFX::EffectLibrary::Instance->Find(weaponEffects->GetEffectName(VTOL_DUST_CLOUD));
					
					if (gosEffectSpec)
					{
						dustCloud[commanderID] = gosFX::EffectLibrary::Instance->MakeEffect(gosEffectSpec->m_effectID, flags);
						gosASSERT(dustCloud[commanderID] != NULL);
						
						MidLevelRenderer::MLRTexturePool::Instance->LoadImages();
					}
				}
			}
		}
		else						//IS a recovery vehicle.  DO NOT create VTOL, use KARNOV instead.  NO Vehicle created either
		{
			if (supportID == -1) 
			{
				if (MPlayer && !reinforcePos) 	//Probably same as above here, too.
				{
					vPos[commanderID] = vtolPos;
					if (!salvageTarget)
						STOP(("MissionGUI.beginvtol: null target PASSED IN."));
					char* newPilotName = (char*)LogisticsData::instance->getBestPilot( salvageTarget->tonnage );
					if (vehicleID[commanderID] != 147)
						STOP(("missiongui.beginvtol: bad vehicleID"));
					MPlayer->sendReinforcement(vehicleID[commanderID], salvageTarget->netRosterIndex, newPilotName, commanderID, vPos[commanderID], 3);
					return;
				}
			}

			AppearanceType* appearanceType = appearanceTypeList->getAppearance( BLDG_TYPE << 24, "karnov" );
			vTol[commanderID] = new BldgAppearance;
			vTol[commanderID]->init( appearanceType );
			
			if (MPlayer)
				mechToRecover[commanderID] = MPlayer->moverRoster[MPlayer->reinforcements[commanderID][1]];
			else
				mechToRecover[commanderID] = salvageTarget;
			mechRecovered[commanderID] = false;	//Need to know when mech is done so Karnov can fly away.
			
			if (!dustCloud[commanderID] && g_userPreferences.useNonWeaponEffects)
			{
				if (strcmp(weaponEffects->GetEffectName(KARNOV_DUST_CLOUD),"NONE") != 0)
				{
					//--------------------------------------------
					// Yes, load it on up.
					unsigned flags = 0;
		
					Check_Object(gosFX::EffectLibrary::Instance);
					gosFX::Effect::Specification* gosEffectSpec = gosFX::EffectLibrary::Instance->Find(weaponEffects->GetEffectName(KARNOV_DUST_CLOUD));
					
					if (gosEffectSpec)
					{
						dustCloud[commanderID] = gosFX::EffectLibrary::Instance->MakeEffect(gosEffectSpec->m_effectID, flags);
						gosASSERT(dustCloud[commanderID] != NULL);
						
						MidLevelRenderer::MLRTexturePool::Instance->LoadImages();
					}
				}
			}
			
			if (!recoveryBeam[commanderID])
			{
				if (strcmp(weaponEffects->GetEffectName(KARNOV_RECOVERY_BEAM),"NONE") != 0)
				{
					//--------------------------------------------
					// Yes, load it on up.
					unsigned flags = 0;
		
					Check_Object(gosFX::EffectLibrary::Instance);
					gosFX::Effect::Specification* gosEffectSpec = gosFX::EffectLibrary::Instance->Find(weaponEffects->GetEffectName(KARNOV_RECOVERY_BEAM));
					
					if (gosEffectSpec)
					{
						recoveryBeam[commanderID] = gosFX::EffectLibrary::Instance->MakeEffect(gosEffectSpec->m_effectID, flags);
						gosASSERT(recoveryBeam[commanderID] != NULL);
						
						MidLevelRenderer::MLRTexturePool::Instance->LoadImages();
					}
				}
			}
		}
	}

	if (commanderID == Commander::home->getId())
	{
		switch (vehicleID[commanderID])
		{
			case 120:	//Minelayer
				g_soundSystem->playSupportSample(SUPPORT_MINELAYER);
			break;

			case 182:	//Repair
				g_soundSystem->playSupportSample(SUPPORT_REPAIR);
			break;

			case 393:	//Scout
				g_soundSystem->playSupportSample(SUPPORT_SCOUT);
			break;

			case 147:	//Recovery
				g_soundSystem->playSupportSample(SUPPORT_RECOVER);
			break;

			case 415:	//Artillery
				g_soundSystem->playSupportSample(SUPPORT_ARTILLERY);

			break;
		}
	}

	Stuff::Vector3D newPos = vtolPos;
	float rotation = 0.f;  // this needs to be pointing 180 degrees from drop point
	
	if (vehicleID[commanderID] == 147)
	{
		//Move wPos to be over the cockpit of the downed mech.
		if (mechToRecover[commanderID] && mechToRecover[commanderID]->appearance)
			newPos = mechToRecover[commanderID]->appearance->getNodeNamePosition("cockpit");
	}
	
	if (vehicleID[commanderID] != 147)
		g_soundSystem->playDigitalSample(VTOL_ANIMATE,newPos);
	else
		g_soundSystem->playDigitalSample(SALVAGE_CRAFT,newPos);
	
	vTol[commanderID]->setObjectParameters( newPos, 0, false, Team::home->id, 0);
	eye->update();
	vTol[commanderID]->update();
	vTol[commanderID]->setInView( true );
	vTol[commanderID]->setVisibility( 1, 1 );
	vTol[commanderID]->rotation = rotation;
	
	if (vehicleID[commanderID] != 147)
		vTol[commanderID]->setGesture(0);		//Start Animation at beginning
	else
		vTol[commanderID]->setGesture(1);		//Start Animation with Landing -- KARNOV

	if (dustCloud[commanderID])
	{
		Stuff::LinearMatrix4D 	shapeOrigin;
		Stuff::LinearMatrix4D	localToWorld;
		Stuff::LinearMatrix4D	localResult;
					
		Stuff::Vector3D dustPos = vTol[commanderID]->position;
		Stuff::Point3D wakePos;
		wakePos.x = -dustPos.x;
		wakePos.y = dustPos.z;
		wakePos.z = dustPos.y;
			
		shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
		shapeOrigin.BuildTranslation(wakePos);
					
		gosFX::Effect::ExecuteInfo info((Stuff::Time)g_missionTime,&shapeOrigin,NULL);
	
		dustCloud[commanderID]->SetLoopOff();
		dustCloud[commanderID]->SetExecuteOn();
		dustCloud[commanderID]->Start(&info);
	}
				
  	paintingVtol[commanderID] = 1;
	vTolSpeed = -75.f;

	vPos[commanderID] = vtolPos;

	vehicleDropped[commanderID] = false;
	
	if ( !MPlayer || commanderID == MPlayer->commanderID )
		controlGui.disableAllVehicleButtons();
}

bool MissionInterfaceManager::canSalvage( GameObject* pMover )
{
	bool lineOfSight = Team::home->teamLineOfSight(pMover->getPosition(),0.0f);

	return (controlGui.forceGroupBar.getIconCount() < MAX_ICONS
		&& target->isDisabled() && lineOfSight && 
		target->getObjectClass() == BATTLEMECH ) ? 1 : 0;
}

void MissionInterfaceManager::doSalvage()
{
	if ( canSalvage(target) )
	{
		TacticalOrder tacOrder;
	
		tacOrder.init(ORDER_ORIGIN_PLAYER, TACTICAL_ORDER_RECOVER );
		tacOrder.targetWID = target->getWatchID();
		tacOrder.attackParams.type = ATTACK_NONE;
		tacOrder.attackParams.method = ATTACKMETHOD_RAMMING;
		tacOrder.attackParams.pursue = true;
		tacOrder.moveParams.wayPath.mode[0] = TRAVEL_MODE_FAST;

		handleOrders( tacOrder );
	}
}

bool MissionInterfaceManager::canRepair( GameObject* pMover )
{
	if ( !pMover )
		return 0;
	
	if ( !pMover->isMover() )
		return 0;

	if ( Team::home->isEnemy( pMover->getTeam() ) )
		return 0;

	if (pMover->isDisabled() || pMover->isDestroyed())
		return 0;

	// find the selected guy
	int watchID = 0;
	Team* pTeam = Team::home;
	for (long i = 0; i < pTeam->getRosterSize(); i++)
	{
		Mover* pTmpMover = (Mover*)pTeam->getMover( i );
		if ( pTmpMover->isSelected() && pTmpMover->getCommander()->getId() == Commander::home->getId() )
		{
			if ( !watchID )
				watchID = pTmpMover->getWatchID();
			else
				watchID = -1;
		}
	}
	
	if  ( ((Mover*)pMover)->needsRefit() || ( ((Mover*)pMover)->refitBuddyWID == watchID && watchID) )
		return true;

	return 0;
}
void MissionInterfaceManager::doRepair(GameObject* who)
{
	TacticalOrder tacOrder;
	tacOrder.init(ORDER_ORIGIN_PLAYER, TACTICAL_ORDER_REFIT, false);
	tacOrder.targetWID = who->getWatchID();
	tacOrder.selectionIndex = -1;
	tacOrder.moveParams.wayPath.mode[0] = controlGui.getWalk() ?  TRAVEL_MODE_SLOW : TRAVEL_MODE_FAST;
	tacOrder.moveParams.faceObject = true;
	tacOrder.moveParams.fromArea = -1;
	tacOrder.moveParams.wait = false;

	handleOrders( tacOrder );

	g_soundSystem->playDigitalSample(BUTTON5);
	controlGui.setDefaultSpeed();
}

bool MissionInterfaceManager::canRepairBay( GameObject* bay)
{
	if (bay && bay->getRefitPoints() > 0.0f)
	{
		Team* pTeam = Team::home;
		for (long i = 0; i < pTeam->getRosterSize(); i++)
		{
			Mover* pMover = (Mover*)pTeam->getMover( i );
			if ( pMover->isSelected() && pMover->needsRefit() && pMover->getCommander()->getId() == Commander::home->getId() )
			{
				return true;
			}
		}
	}

	return false;
}
void MissionInterfaceManager::doRepairBay(GameObject* who)
{
	TacticalOrder tacOrder;
	tacOrder.init(ORDER_ORIGIN_PLAYER, TACTICAL_ORDER_GETFIXED, false);
	tacOrder.targetWID = who->getWatchID();
	tacOrder.selectionIndex = -1;
	tacOrder.moveParams.wayPath.mode[0] = controlGui.getWalk() ?  TRAVEL_MODE_SLOW : TRAVEL_MODE_FAST;
	tacOrder.moveParams.faceObject = true;
	tacOrder.moveParams.fromArea = -1;
	tacOrder.moveParams.wait = false;

	handleOrders( tacOrder );

	g_soundSystem->playDigitalSample(BUTTON5);
	controlGui.setDefaultSpeed();
}

int MissionInterfaceManager::vehicleCommand()
{
	if ( controlGui.getVehicleCommand() )
		controlGui.setVehicleCommand( false );
	else
		controlGui.setVehicleCommand( true );

	return 0;
}

int MissionInterfaceManager::showObjectives()
{
	controlGui.startObjectives( !controlGui.objectivesStarted( ) );
	return 0;
}

int MissionInterfaceManager::showObjectives(bool on)
{
	controlGui.startObjectives( on );
	return 0;
}

int MissionInterfaceManager::togglePause()
{
	if ( !bPausedWithoutMenu )
	{
		bPaused ^= 1;
	}
	else
		bPausedWithoutMenu = 0;

	g_soundSystem->playDigitalSample( LOG_NEXTBACKBUTTONS );

	if ( bPaused )
	{
		controlGui.beginPause();
	}
	else
	{
		controlGui.endPause();
	}

	return 1;
}

int MissionInterfaceManager::togglePauseWithoutMenu()
{
	bPausedWithoutMenu;

	if ( !bPausedWithoutMenu )
	{
		if ( !bPaused )
		{
			bPausedWithoutMenu = 1;
			bPaused = 1;
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		gosASSERT(bPaused);
		bPausedWithoutMenu = 0;
		bPaused = 0;
		return 1;
	}
}

bool MissionInterfaceManager::isPaused()
{
	return bPaused;
}

bool MissionInterfaceManager::isPausedWithoutMenu()
{
	return bPausedWithoutMenu;
}

void	MissionInterfaceManager::swapResolutions()
{
	controlGui.swapResolutions( Environment.screenWidth );
	resolution = Environment.screenWidth;
	keyboardRef->init();
}

int MissionInterfaceManager::switchTab()
{
	controlGui.switchTabs(1);		
	return 1;
}

int MissionInterfaceManager::reverseSwitchTab()
{
	controlGui.switchTabs(-1);		
	return 1;
}

int MissionInterfaceManager::infoCommand()
{
	if ( !controlGui.infoButtonPressed() )
		controlGui.pressInfoButton();
	return 1;
}

int MissionInterfaceManager::infoButtonReleased()
{
	if ( controlGui.infoButtonPressed() )
		controlGui.pressInfoButton();

	return 1;
}
int MissionInterfaceManager::energyWeapons()
{
	if ( gos_GetKeyStatus( (gosEnum_KeyIndex)(s_commands[ENERGY_WEAPON_INDEX].key & 0x0000ffff) ) == KEY_HELD )
		bEnergyWeapons = 1;	
	else
		bEnergyWeapons = 0;
	return 0;
}
int MissionInterfaceManager::sendAirstrike()
{
	controlGui.pressAirstrikeButton();
	return 1;
}
int MissionInterfaceManager::sendLargeAirstrike()
{
	controlGui.pressLargeAirstrikeButton();
	return 1;
}
int MissionInterfaceManager::gotoNextNavMarker()
{
	// need to find the next nav marker....
	for ( CObjectives::EIterator iter = Team::home->objectives.Begin();
		!iter.IsDone(); iter++ )
		{
			if ( (*iter)->Status(Team::home->objectives) == OS_UNDETERMINED  && (*iter)->DisplayMarker() == NAV )
			{
				Stuff::Vector3D pos;
				pos.x = (*iter)->MarkerX();
				pos.y = (*iter)->MarkerY();
				if ( !pos.x && !pos.y )
					continue;
				pos.z = land->getTerrainElevation( pos );
				LocationNode path;
				path.location = pos;
				path.run = true;
				path.next = NULL;

				TacticalOrder tacOrder;
				tacOrder.init(ORDER_ORIGIN_PLAYER, TACTICAL_ORDER_MOVETO_POINT, false);
				tacOrder.initWayPath(&path);
				tacOrder.moveParams.wait = false;
				tacOrder.moveParams.wayPath.mode[0] =  controlGui.getWalk() ? TRAVEL_MODE_SLOW : TRAVEL_MODE_FAST;
				if ( controlGui.getMines() )
					tacOrder.moveParams.mode = MOVE_MODE_MINELAYING;
				tacOrder.pack(NULL, NULL);

				handleOrders(tacOrder);
				
				g_soundSystem->playDigitalSample(BUTTON5);

				return 1;
			}
		}

	// if we got here there weren't any valid markers, play bad sound
	g_soundSystem->playDigitalSample( INVALID_GUI );
	return 0;
}
int MissionInterfaceManager::sendSensorStrike()
{
	controlGui.pressSensorStrikeButton();
	return 1;
}

int MissionInterfaceManager::quickDebugInfo() {

	#ifndef FINAL
	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) {
		long row, col;
		land->worldToCell(wPos, row, col);
		char debugString[256];
		if (target)
			sprintf(debugString, "INFO = %s(%c%c) %d(W%d) [%ld, %ld] (%.4f, %.4f, %.4f) #Pth=%d(%d) OBJ = %s\n",
				terrainStr[GameMap->getTerrain(row, col)],
				GameMap->getPassable(row, col) ? 'T' : 'F',
				GameMap->getForest(row, col) ? 'T' : 'F',
				GlobalMoveMap[target->getMoveLevel()]->calcArea(row, col),
				GameMap->getShallowWater(row, col) ? 1 : (GameMap->getDeepWater(row, col) ? 2 : 0),
				row, col,
				wPos.x, wPos.y, wPos.z,
				PathManager->numPaths, PathManager->peakPaths, target->getName());
		else
			sprintf(debugString, "INFO = %s(%c%c) %d(W%d) [%ld, %ld] (%.4f, %.4f, %.4f) #Pth=%d(%d)\n",
				terrainStr[GameMap->getTerrain(row, col)],
				GameMap->getPassable(row, col) ? 'T' : 'F',
				GameMap->getForest(row, col) ? 'T' : 'F',
				GlobalMoveMap[0]->calcArea(row, col),
				GameMap->getShallowWater(row, col) ? 1 : (GameMap->getDeepWater(row, col) ? 2 : 0),
				row, col,
				wPos.x, wPos.y, wPos.z,
				PathManager->numPaths, PathManager->peakPaths);
		DEBUGWINS_print(debugString);
		sprintf(debugString, "REQ =");
		char s[10];
		for (long i = 0; i < 25; i++) {
			sprintf(s, " %02d", PathManager->sourceTally[i]);
			strcat(debugString, s);
		}
		DEBUGWINS_print(debugString);
		lastTime = gos_GetElapsedTime();
	}
	#endif
	return(1);
}

int MissionInterfaceManager::setGameObjectWindow() {
	
	#ifndef FINAL
	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) {
		DEBUGWINS_setGameObject(-1, target);
		lastTime = gos_GetElapsedTime();
	}	
	#endif
	return(1);
}

int MissionInterfaceManager::pageGameObjectWindow1() {

	#ifndef FINAL
	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) {
		if (DebugGameObject[0] && DebugGameObject[0]->isMover())
			((MoverPtr)DebugGameObject[0])->debugPage++;
		lastTime = gos_GetElapsedTime();
	}
	#endif
	return(1);
}

int MissionInterfaceManager::pageGameObjectWindow2() {

	#ifndef FINAL
	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) {
		if (DebugGameObject[1] && DebugGameObject[1]->isMover())
			((MoverPtr)DebugGameObject[1])->debugPage++;
		lastTime = gos_GetElapsedTime();
	}
	#endif
	return(1);
}

int MissionInterfaceManager::pageGameObjectWindow3() {

	#ifndef FINAL
	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) {
		if (DebugGameObject[2] && DebugGameObject[2]->isMover())
			((MoverPtr)DebugGameObject[2])->debugPage++;
		lastTime = gos_GetElapsedTime();
	}
	#endif
	return(1);
}

int MissionInterfaceManager::rotateObjectLeft () {

	#ifndef FINAL
	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) {
		if (target)
			target->rotate(4.0);
		lastTime = gos_GetElapsedTime();
	}
	#endif
	return(1);
}

int MissionInterfaceManager::rotateObjectRight () {

	#ifndef FINAL
	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) {
		if (target)
			target->rotate(-4.0);
		lastTime = gos_GetElapsedTime();
	}
	#endif
	return(1);
}

int MissionInterfaceManager::jumpToDebugGameObject1() {

	#ifndef FINAL
	DEBUGWINS_viewGameObject(0);
	#endif
	return(1);
}

int MissionInterfaceManager::jumpToDebugGameObject2() {

	#ifndef FINAL
	DEBUGWINS_viewGameObject(1);
	#endif
	return(1);
}

int MissionInterfaceManager::jumpToDebugGameObject3() {

	#ifndef FINAL
	DEBUGWINS_viewGameObject(2);
	#endif
	return(1);
}

int MissionInterfaceManager::toggleDebugWins() {

	#ifndef FINAL
	static long debugWinsState = 0;
	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) {
		debugWinsState = (++debugWinsState % 6);
		bool debugStateFlags[6][7] = {
			{false, false, false, false, false, false, false},
			{false, false, false, false, false, true, false},
			{false, false, false, false, false, true, true},
			{false, true, true, true, false, true, true},
			{true, true, true, true, false, true, true},
			{false, false, false, false, true, false, true}
		};
		DEBUGWINS_display(debugStateFlags[debugWinsState]);
		lastTime = gos_GetElapsedTime();
	}
	#endif
	return(1);
}

extern bool g_dbgShowMovers;
int MissionInterfaceManager::showMovers() {

	#ifndef FINAL
	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) {
		g_dbgShowMovers = !g_dbgShowMovers;
		lastTime = gos_GetElapsedTime();
	}
	#endif
	return(1);
}

int MissionInterfaceManager::zeroHPrime() {

	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) {
		ZeroHPrime = !ZeroHPrime;
		lastTime = gos_GetElapsedTime();
	}
	return(1);
}

int MissionInterfaceManager::calcValidAreaTable() {

	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) {
		CalcValidAreaTable = !CalcValidAreaTable;
		lastTime = gos_GetElapsedTime();
	}
	return(1);
}

int MissionInterfaceManager::teleport() {
	
	#ifndef FINAL
	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) {
		Team* pTeam = Team::home;
		Stuff::Vector3D moveGoals[MAX_MOVERS];
		long numMovers = 0;
		for (long i = 0; i < pTeam->getRosterSize(); i++) {
			Mover* mover = pTeam->getMover(i);
			if (mover->isSelected() && mover->getCommander()->getId() == Commander::home->getId())
				numMovers++;
		}
		MoverGroup::calcMoveGoals(wPos, numMovers, moveGoals);
		numMovers = 0;
		for (i = 0; i < pTeam->getRosterSize(); i++) {
			Mover* mover = (Mover*)pTeam->getMover( i );
			if (mover->isSelected() && mover->getCommander()->getId() == Commander::home->getId() )
				((MoverPtr)mover)->setTeleportPosition(moveGoals[numMovers++]);
		}
		lastTime = gos_GetElapsedTime();
	}
	#endif
	return(1);
}

extern GameLog*	LRMoveLog;

extern bool g_quitGame;

int MissionInterfaceManager::globalMapLog () {

	#ifndef FINAL
	static double lastTime = 0.0;
	if (MPlayer)		//Otherwise we crash in singlePlayer!!
	{
		if ((lastTime + 0.5) < gos_GetElapsedTime()) 
		{
			//MPlayer->leaveSession();
			//g_quitGame = true;
			//GlobalMap::toggleLog();
			lastTime = gos_GetElapsedTime();
		}
	}
	#endif
	return(1);
}

int MissionInterfaceManager::brainDead () {

	#ifndef FINAL
	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) {
		for (long i = 1; i < MAX_TEAMS; i++)
			MechWarrior::brainsEnabled[i] = !MechWarrior::brainsEnabled[i];
		lastTime = gos_GetElapsedTime();
	}
	#endif
	return(1);
}

int MissionInterfaceManager::goalPlan() {
	
	#ifndef FINAL
	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) {
		Team* pTeam = Team::home;
		long numMovers = 0;
		for (long i = 0; i < pTeam->getRosterSize(); i++) {
			Mover* mover = pTeam->getMover(i);
			if (mover->isSelected() && mover->getCommander()->getId() == Commander::home->getId())
				numMovers++;
		}
		for (i = 0; i < pTeam->getRosterSize(); i++) {
			Mover* mover = (Mover*)pTeam->getMover( i );
			if (mover->isSelected() && mover ->getCommander()->getId() == Commander::home->getId()) {
				((MoverPtr)mover)->getPilot()->setUseGoalPlan(!((MoverPtr)mover)->getPilot()->getUseGoalPlan());
				((MoverPtr)mover)->getPilot()->setMainGoal(GOAL_ACTION_NONE, NULL, NULL, -1.0);
			}
		}
		lastTime = gos_GetElapsedTime();
	}
	#endif
	return(1);
}

int MissionInterfaceManager::enemyGoalPlan() {
	
	#ifndef FINAL
	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) {
		g_dbgGoalPlanEnemy = !g_dbgGoalPlanEnemy;
		for (long i = 0; i < ObjectManager->getNumMovers(); i++) {
			Mover* mover = ObjectManager->getMover(i);
			if (mover->getTeam() != Team::home) {
				((MoverPtr)mover)->getPilot()->setUseGoalPlan(g_dbgGoalPlanEnemy);
				((MoverPtr)mover)->getPilot()->setMainGoal(GOAL_ACTION_NONE, NULL, NULL, -1.0);
			}
		}
		lastTime = gos_GetElapsedTime();
	}
	#endif
	return(1);
}

int MissionInterfaceManager::showVictim () {
	
	#ifndef FINAL
	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) 
	{
		if (DebugGameObject[0] && DebugGameObject[0]->isMover()) 
		{
			GameObjectPtr targetObj = ((MoverPtr)DebugGameObject[0])->getPilot()->getCurTacOrder()->getTarget();
			if (targetObj)
				DEBUGWINS_setGameObject(-1, targetObj);
		}
		lastTime = gos_GetElapsedTime();
	}	
	#endif
	return(1);
}

void damageObject (GameObjectPtr victim, float damage) {

	WeaponShotInfo shot;
	switch (victim->getObjectClass()) {
		case BATTLEMECH:
		case GROUNDVEHICLE:
			shot.init(NULL, -2, damage, victim->calcHitLocation(NULL,-1,ATTACKSOURCE_WEAPONFIRE,0), 0);
			victim->handleWeaponHit(&shot, (MPlayer != NULL));
			break;
		default:
			shot.init(NULL, -1, damage, 0, 0);
			victim->handleWeaponHit(&shot, (MPlayer != NULL));
			break;
	}
}

int MissionInterfaceManager::damageObject1 () 
{
	#ifndef FINAL
	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) 
	{
		if (target)
			target->repairAll();

		lastTime = gos_GetElapsedTime();
	}
	#endif
	return(1);
}

int MissionInterfaceManager::damageObject2 () {

	#ifndef FINAL
	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) {
		if (target)
			damageObject(target, 4.0);
		lastTime = gos_GetElapsedTime();
	}
	#endif
	return(1);
}

int MissionInterfaceManager::damageObject3 () {

	#ifndef FINAL
	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) {
		if (target)
			damageObject(target, 9.0);
		lastTime = gos_GetElapsedTime();
	}
	#endif
	return(1);
}

int MissionInterfaceManager::damageObject4 () {

	#ifndef FINAL
	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) {
		if (target)
			damageObject(target, 16.0);
		lastTime = gos_GetElapsedTime();
	}
	#endif
	return(1);
}

int MissionInterfaceManager::damageObject5 () {

	#ifndef FINAL
	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) {
		if (target)
			damageObject(target, 25.0);
		lastTime = gos_GetElapsedTime();
	}
	#endif
	return(1);
}

int MissionInterfaceManager::damageObject6 () {

	#ifndef FINAL
	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) {
		if (target)
			damageObject(target, 36.0);
		lastTime = gos_GetElapsedTime();
	}
	#endif
	return(1);
}

int MissionInterfaceManager::damageObject7 () {

	#ifndef FINAL
	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) {
		if (target)
			damageObject(target, 49.0);
		lastTime = gos_GetElapsedTime();
	}
	#endif
	return(1);
}

int MissionInterfaceManager::damageObject8 () {

	#ifndef FINAL
	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) {
		if (target)
			damageObject(target, 64.0);
		lastTime = gos_GetElapsedTime();
	}
	#endif
	return(1);
}

int MissionInterfaceManager::damageObject9 () {

	#ifndef FINAL
	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) {
		if (target)
			damageObject(target, 81);
		lastTime = gos_GetElapsedTime();
	}
	#endif
	return(1);
}

int MissionInterfaceManager::damageObject0 () {

	#ifndef FINAL
	static double lastTime = 0.0;
	if ((lastTime + 0.5) < gos_GetElapsedTime()) 
	{
		if (target)
		{
			if (target->isMover())
				((MoverPtr)target)->disable(0);
			else
				damageObject(target, 100.0);
		}

		lastTime = gos_GetElapsedTime();
	}
	#endif
	return(1);
}

int MissionInterfaceManager::toggleCompass()
{
	((GameCamera*)eye)->toggleCompass();
	return 0;
}

bool MissionInterfaceManager::selectionIsHelicopters( )
{
	Team* pTeam = Team::home;
   	for (long i=0;i<pTeam->getRosterSize();i++)
   	{
   		Mover* pMover = (Mover*)pTeam->getMover( i );
   		if ( pMover->isSelected() && pMover->getCommander()->getId() == Commander::home->getId() )
   		{
			if ( pMover->getMoveType() != MOVETYPE_AIR )
			{
				return 0;
			}
   		}
   	}

	return true;
}

int MissionInterfaceManager::saveHotKeys( FitIniFile& file )
{
	file.writeBlock( "Keyboard" );
	
	file.writeIdLong("WayPointKey", s_waypointKey);
	for (int i = 0; i < s_numCommands; i++)
	{
		if (s_commands[i].hotKeyDescriptionText == -1) // MCHD CHANGE (02/14/15): If the user can't change it, don't save it
			continue;

		file.writeIdLong( s_commands[i].name, s_commands[i].key );	// MCHD CHANGE (02/14/15): Save the names	
	}

	return 0;
}
int MissionInterfaceManager::loadHotKeys( FitIniFile& file )
{
	static bool defaultKeysLoaded = false;
	if (defaultKeysLoaded == false)
	{
		for ( int i = 0; i < s_numCommands; i++ )
		{
			s_defaultKeys[i] = s_commands[i].key;		
		}
		defaultKeysLoaded = true;
	}

	if ( NO_ERR == file.seekBlock( "Keyboard" ) )
	{
		long tmp;
		file.readIdLong("WayPointKey", tmp);
		s_waypointKey = gosEnum_KeyIndex(tmp);

		bool warning = false;
		for (int i = 0; i < s_numCommands; i++)
		{
			if (s_chatCommandIndex == -1 && strcmp(s_commands[i].name, "AllChat") == 0 || strcmp(s_commands[i].name, "AllChat") == 0)
				s_chatCommandIndex = i;

			if (s_commands[i].hotKeyDescriptionText == -1) // MCHD CHANGE (02/14/15): Don't try to read in permanently bound commands
				continue;

			if (file.readIdLong(s_commands[i].name, s_commands[i].key) == VARIABLE_NOT_FOUND)
				s_commands[i].key = s_defaultKeys[i];
		}

		// Prevent double-binding keys
		for (int i = 0; i < s_numCommands; i++)
		{
			// Check normal commands for duplicate bindings
			bool reset = false;
			int resetIndex = 0;
			for (int j = 0; j < s_numCommands; ++j)
			{
				if (i == j)
					continue;

				if (s_commands[i].key == s_commands[j].key)
				{
					// If the j key is default, set the i key to its default
					if (s_commands[j].key == s_defaultKeys[j])
					{
						s_commands[i].key = s_defaultKeys[i];
						warning = reset = true;
						resetIndex = i;
						break;
					}
					// If the i key is default, set the j key to its default
					else
					{
						s_commands[j].key = s_defaultKeys[j];
						warning = reset = true;
						resetIndex = j;
						break;
					}
				}
			}

#ifndef FINAL
			// A retail user could have over-written debug commands; reset to original bindings if so
			for (int j = 0; j < s_numDebugCommands; ++j)
			{
				if (s_commands[i].key == s_debugCommands[j].key)
				{
					s_commands[i].key = s_defaultKeys[i];
					warning = reset = true;
					resetIndex = i;
					break;
				}
			}
#endif

			// If a command was reset, make sure it's not now in conflict with another regular command
			// If it is, reset that to its default as well and keep doing that until no more keys are in conflict
			while (reset)
			{
				reset = false;
				for (int j = 0; j < s_numCommands; ++j)
				{
					// Permanently bound commands will never have changed or need to be reset
					if (s_commands[j].hotKeyDescriptionText == -1 || j == resetIndex)
						continue;

					if (s_commands[j].key == s_commands[resetIndex].key)
					{
						s_commands[j].key = s_defaultKeys[j];
						reset = true;
						resetIndex = j;
						break;
					}
				}
			}
		}

		if (warning)
		{
			// Error message
			PAUSE(("There were conflicting key bindings and some were reverted to their original settings."));
		}

		return 0;
	}

	return -1;
}

// Returns whether or not this key is valid for re-binding 
// (false == okay to re-bind; true == permanently bound)
bool MissionInterfaceManager::isHotKeyLocked(long _key)
{
	// Reserved for groups
	if ((_key & 0xffff) >= KEY_0 && (_key & 0xffff) <= KEY_9)
	{
		return true;
	}
	// Non-viewable commands should not be changed in the background
	for (int i = 0; i < MissionInterfaceManager::s_numCommands; ++i)
	{
		if (MissionInterfaceManager::s_commands[i].hotKeyDescriptionText == -1)
		{
			if (_key == MissionInterfaceManager::s_commands[i].key)
			{
				return true;
			}
		}
	}
#ifndef FINAL
	for (int i = 0; i < MissionInterfaceManager::s_numDebugCommands; ++i)
	{
		if (_key == MissionInterfaceManager::s_debugCommands[i].key)
		{
			return true;
		}
	}
#endif

	return false;
}

int MissionInterfaceManager::setHotKey(int whichCommand, gosEnum_KeyIndex newKey, bool bShift, bool bControl, bool bAlt)
{
	gosASSERT(whichCommand < s_numCommands);
	long oldKey = s_commands[whichCommand].key;
	long key = newKey;
	if (bShift)
		key |= SHIFT;
	if (bControl)
		key |= CTRL;
	if (bAlt)
		key |= ALT;

	if (s_commands[whichCommand].key & WAYPT)
	{
		key |= WAYPT;
	}
	else
	{
		if (key != oldKey)
		{
			// change corresponding waypoint keys
			for (int i = 0; i < s_numCommands; i++)
			{
				long currentKey = s_commands[i].key;
				if (((currentKey & 0x0000ffff) == oldKey) && (currentKey & WAYPT))
				{
					s_commands[i].key = key | WAYPT;
				}
			}
		}
	}

	s_commands[whichCommand].key = key;

	return 0;
}

int MissionInterfaceManager::getHotKey( int whichCommand, gosEnum_KeyIndex& newKey, bool& bShift, bool& bControl, bool& bAlt )
{
	gosASSERT( whichCommand < s_numCommands );
	
	long key = s_commands[whichCommand].key;

	if ( key & SHIFT )
		bShift = true;
		
	if ( key & CTRL )
		bControl = true;
	
	if ( key & ALT )
		bAlt = true;

	key &= 0x0000ffff;
	newKey = (gosEnum_KeyIndex)key;

	return 0;
}

int MissionInterfaceManager::setWayPointKey( gosEnum_KeyIndex key )
{
	s_waypointKey = key;

	return 0;
}

void MissionInterfaceManager::setAOEStyle()
{
	g_userPreferences.leftRightMouseProfile = true;
}
void MissionInterfaceManager::setMCStyle()
{
	g_userPreferences.leftRightMouseProfile = false;
}

bool MissionInterfaceManager::isAOEStyle()
{
	return g_userPreferences.leftRightMouseProfile == true;
}
bool MissionInterfaceManager::isMCStyle()
{
	return g_userPreferences.leftRightMouseProfile == false;
}

void MissionInterfaceManager::doEject( GameObject* who )
{
	TacticalOrder tacOrder;
	tacOrder.init(ORDER_ORIGIN_PLAYER, TACTICAL_ORDER_EJECT, true);
	tacOrder.pack(NULL, NULL);
	MoverPtr pMover = (MoverPtr)who;
	if (MPlayer && !MPlayer->isServer())
		MPlayer->sendPlayerOrder(&tacOrder, false, 1, &pMover);
	else
		pMover->handleTacticalOrder(tacOrder);
}

bool MissionInterfaceManager::hotKeyIsPressed( int whichCommand )
{
	long key = s_commands[whichCommand].key;
	bool bShift = 0;
	bool bControl = 0;
	bool bAlt = 0;

	if ( key & SHIFT )
		bShift = true;
		
	if ( key & CTRL )
		bControl = true;
	
	if ( key & ALT )
		bAlt = true;

	key &= 0x0000ffff;

	bool shiftDn = userInput->shift();
	bool ctrlDn = userInput->ctrl();
	bool altDn = userInput->alt();

	if ( gos_GetKeyStatus( (gosEnum_KeyIndex)(key&0x000000ff) ) == KEY_PRESSED ||
		gos_GetKeyStatus( (gosEnum_KeyIndex)(key&0x000000ff) ) == KEY_HELD )
	{
		if ( shiftDn == bShift 
			&& ctrlDn == bControl
			&& altDn == bAlt )
		{
			return true;
		}
	}

	return false;
}

void testKeyStuff()
{
	bool bShift, bCtrl, bAlt;
	gosEnum_KeyIndex key = KEY_F9;
	MissionInterfaceManager::instance()->getHotKey( 5, key, bShift, bCtrl, bAlt );
	MissionInterfaceManager::instance()->setHotKey( 5, KEY_L, 0, 0, 0 );
	MissionInterfaceManager::instance()->getHotKey( 6, key, bShift, bCtrl, bAlt );

	FitIniFile file;
	file.open( "keyboards.fit" );
	MissionInterfaceManager::instance()->loadHotKeys( file );
	MissionInterfaceManager::instance()->getHotKey( 6, key, bShift, bCtrl, bAlt );
}

int MissionInterfaceManager::toggleHotKeys()
{
	bDrawHotKeys ^= 1;
	if ( bDrawHotKeys )
	{
		keyboardRef->reseed( s_commands );

		if ( !isPaused() )
			togglePauseWithoutMenu();
	}
	else if ( !bDrawHotKeys && isPaused() && isPausedWithoutMenu() )
		togglePauseWithoutMenu();
	return 1;
}

void MissionInterfaceManager::drawHotKeys()
{

	keyboardRef->render();
	return;
}

void MissionInterfaceManager::drawHotKey( const char* keyString, const char* descString, long x, long y )
{
	hotKeyFont.render( keyString, 0, y, x, Environment.screenHeight, 0xffffffff, 0, 1 );

	hotKeyFont.render( descString,  x + (10 * Environment.screenWidth/640.f), y, 
		Environment.screenWidth, Environment.screenHeight, 0xffffffff, 0, 0 );
}

void MissionInterfaceManager::doGuardTower()
{

	for (long i = 0; i < Team::home->getRosterSize(); i++)
	{
		Mover* pMover = Team::home->getMover( i );
		if ( pMover->isSelected() && pMover->getCommander()->getId() == Commander::home->getId())
		{
			pMover->suppressionFire = true;
		}
	}
			
	TacticalOrder tacOrder;
	tacOrder.init(ORDER_ORIGIN_PLAYER, TACTICAL_ORDER_ATTACK_POINT);
	//tacOrder.init(ORDER_ORIGIN_PLAYER,  target ? TACTICAL_ORDER_ATTACK_OBJECT : TACTICAL_ORDER_ATTACK_POINT);
	//tacOrder.targetWID = target ? target->getWatchID() : 0;
	//if ( !target )
	tacOrder.attackParams.targetPoint = wPos;			
	tacOrder.attackParams.type = bEnergyWeapons ? ATTACK_CONSERVING_AMMO : ATTACK_TO_DESTROY;
	tacOrder.attackParams.method = ATTACKMETHOD_RANGED;
	tacOrder.attackParams.range = FIRERANGE_CURRENT;
	tacOrder.attackParams.pursue = false;
	tacOrder.moveParams.faceObject = true;
	tacOrder.moveParams.fromArea = -1;
	tacOrder.moveParams.wait = false;

	handleOrders( tacOrder );

	controlGui.setDefaultSpeed();
}

int		MissionInterfaceManager::toggleHoldPosition()
{
	controlGui.toggleHoldPosition();

	return 1;
}

int MissionInterfaceManager::handleChatKey()
{
	controlGui.toggleChat(0);
	return 1;
}

int	MissionInterfaceManager::handleTeamChatKey()
{
	controlGui.toggleChat(1);
	return 1;
}

float slopeTest[8] = {0.09849140f, 0.30334668f, 0.53451114f, 0.82067879f, 1.21850353f, 1.87086841f, 3.29655821f, 10.15317039f};
long MissionInterfaceManager::makeMoveCursor( bool bLineOfSite )
{

	long rotation = calcRotation();

	// leigh put these in the file funny
	// OK, rotations go clockwise starting from 180...
	gosASSERT( rotation >= 0 && rotation < 32 );
	return mState_WALK1 + 32 * bLineOfSite + rotation;
}

long MissionInterfaceManager::makeJumpCursor( bool bLineOfSite )
{
	long rotation = calcRotation();

	// leigh put these in the file funny
	// OK, rotations go clockwise starting from 180...
	gosASSERT( rotation >= 0 && rotation < 32 );
	return mState_JUMP1 + 32 * bLineOfSite + rotation;
}

long MissionInterfaceManager::makeRunCursor( bool bLineOfSite )
{
	long rotation = calcRotation();

	// leigh put these in the file funny
	// OK, rotations go clockwise starting from 180...
	gosASSERT( rotation >= 0 && rotation < 32 );
	return mState_RUN1 + 32 * bLineOfSite + rotation;
}

long MissionInterfaceManager::calcRotation()
{
	Stuff::Vector3D pos;
	pos.x = 0.f;
	pos.y = 0.f;
	pos.z = 0.f;
	float count = 0;
	for (long i = 0; i < Team::home->getRosterSize(); i++)
	{
		Mover* pMover = Team::home->getMover( i );
		if ( pMover->isSelected() && pMover->getCommander()->getId() == Commander::home->getId() )
		{
			pos.x += pMover->getPosition().x;
			pos.y += pMover->getPosition().y;
			pos.z += pMover->getPosition().z;
			count += 1.f;
		}
	}

	if ( count && (g_framesSinceMissionStart > 3))
	{
		pos.x /= count;
		pos.y /= count;
		pos.z /= count;
	}
	else
		return mState_NORMAL;

	//Now works by finding the vector in world space between the mouse cursor
	// and the movers.  It then normalizes that vector and multiplies by 200
	// to make large.  It then adds that vector to the camera position and we
	// do the projections based on the camera center (which is always the
	// center of the screen) and the new vector.  Complex but it works!!
	// -fs
	Stuff::Vector3D actualPos;
	actualPos.Subtract(pos, wPos);
	actualPos.z = 0.0f;
	
	float actualLength = actualPos.GetLength();
	if (actualLength > Stuff::SMALL)
		actualPos /= actualLength;
	actualPos *= 200.0f;
	
	Stuff::Vector3D camPos = eye->getPosition();
	
	Stuff::Vector4D screenPosMover;
	Stuff::Vector4D screenPosGoal;
	actualPos.Add(camPos,actualPos);
	eye->projectZ( actualPos, screenPosMover );

	// need to find second position
	eye->projectZ( camPos, screenPosGoal );
	
	//CRAZY ATAN code.  Out for now.
	// -fs
	/*
	// determine angle between the two
	float deltaY = -(screenPosGoal.y - screenPosMover.y);
	float deltaX = screenPosGoal.x - screenPosMover.x;

	float theta = 0;
	if ( deltaY && deltaX )
	{
		theta = atan( deltaY/deltaX );
		theta *= RADS_TO_DEGREES; 
	}

	if ( deltaX < 0 )
		theta += 180.f;

	long rotation = ((theta/360.f * 32.f));
	*/

	//Do it the MC1 way!
	float slope = 0.0f;
	if (fabs(screenPosGoal.x - screenPosMover.x) > Stuff::SMALL)
		slope = fabs(screenPosGoal.y - screenPosMover.y) / fabs(screenPosGoal.x - screenPosMover.x);

	long counter = 0;
	long rotation = 0;
	while (slope > slopeTest[counter] && counter < 8)
		counter++;
	if (screenPosGoal.y < screenPosMover.y)
	{
		if (screenPosGoal.x < screenPosMover.x)	// quadrant 0
			rotation = counter;
		else							// quadrant 1
			rotation = 8 + (8 - counter);
	}
	else
	{
		if (screenPosGoal.x > screenPosMover.x)	// quadrant 2
		{
			rotation = 16 + counter;
		}
		else							// quadrant 3
		{
			rotation = 24 + (8 - counter);
			if (rotation == 32)
				rotation = 0;
		}
	}

	return rotation;
}

//--------------------------------------------------------------------------------------
void MissionInterfaceManager::Save (FitIniFilePtr file)
{
	//If VTOL or Karnov are active AND HAVEN'T FINISHED THEIR OPERATION, save them, what vehicle they are going to drop or what mech they were fixing.
	long vtolNum = Commander::home->getId();

	if (vTol[vtolNum] && paintingVtol[vtolNum] && (!vehicleDropped[vtolNum] && !mechRecovered[vtolNum]))
	{
		//Save 'em
		file->writeBlock("VTOLData");

		file->writeIdFloat("VtolPosX",vPos[vtolNum].x);
		file->writeIdFloat("VtolPosY",vPos[vtolNum].y);
		file->writeIdFloat("VtolPosZ",vPos[vtolNum].z);

		file->writeIdLong("VehicleID",vehicleID[vtolNum]);

		if (mechToRecover[vtolNum])
			file->writeIdLong("SalvageMechWID", mechToRecover[vtolNum]->getWatchID());
		else
			file->writeIdLong("SalvageMechWID", 0);

		file->writeIdLong("VTOLFrame",vTol[vtolNum]->currentFrame);
		file->writeIdLong("VTOLGesture",vTol[vtolNum]->getCurrentGestureId());
	}
}

//--------------------------------------------------------------------------------------
void MissionInterfaceManager::Load (FitIniFilePtr file)
{
	//Is there any VTOL data here?  OK if not!
	long vtolNum = Commander::home->getId();
	long commanderID = vtolNum;

	if (file->seekBlock("VTOLData") == NO_ERR)
	{
		//Load 'em
		file->readIdFloat("VtolPosX",vPos[vtolNum].x);
		file->readIdFloat("VtolPosY",vPos[vtolNum].y);
		file->readIdFloat("VtolPosZ",vPos[vtolNum].z);

		file->readIdLong("VehicleID",vehicleID[vtolNum]);

		if (vehicleID[vtolNum] != 147)		//NOT a recovery vehicle.  Standard VTOL
		{
			//Go ahead and make a VTOL.  We're gonna need it!
			AppearanceType* appearanceType = appearanceTypeList->getAppearance( BLDG_TYPE << 24, "vtol" );
			vTol[commanderID] = new BldgAppearance;
			vTol[commanderID]->init( appearanceType );
		}
		else
		{
			AppearanceType* appearanceType = appearanceTypeList->getAppearance( BLDG_TYPE << 24, "karnov" );
			vTol[commanderID] = new BldgAppearance;
			vTol[commanderID]->init( appearanceType );
		}

		long mechWID = 0;
		file->readIdLong("SalvageMechWID", mechWID);
		mechToRecover[vtolNum] = (MoverPtr)ObjectManager->getByWatchID(mechWID);

		file->readIdFloat("VTOLFrame",vTol[vtolNum]->currentFrame);

		long gestureId = 0;
		file->readIdLong("VTOLGesture",gestureId);
		vTol[vtolNum]->setGesture(gestureId);

		//Now setup everything else.
		// Pretty much same code as beginVtol
		vehicleFile = controlGui.getVehicleNameFromID(vehicleID[vtolNum]);

//		if ( !vTol[vtolNum] )
		{
			if (vehicleID[vtolNum] != 147)		//NOT a recovery vehicle.  Standard VTOL
			{
				if (!dustCloud[commanderID] && g_userPreferences.useNonWeaponEffects)
				{
					if (strcmp(weaponEffects->GetEffectName(VTOL_DUST_CLOUD),"NONE") != 0)
					{
						//--------------------------------------------
						// Yes, load it on up.
						unsigned flags = 0;

						Check_Object(gosFX::EffectLibrary::Instance);
						gosFX::Effect::Specification* gosEffectSpec = gosFX::EffectLibrary::Instance->Find(weaponEffects->GetEffectName(VTOL_DUST_CLOUD));

						if (gosEffectSpec)
						{
							dustCloud[commanderID] = gosFX::EffectLibrary::Instance->MakeEffect(gosEffectSpec->m_effectID, flags);
							gosASSERT(dustCloud[commanderID] != NULL);

							MidLevelRenderer::MLRTexturePool::Instance->LoadImages();
						}
					}
				}
			}
			else						//IS a recovery vehicle.  DO NOT create VTOL, use KARNOV instead.  NO Vehicle created either
			{
				mechRecovered[commanderID] = false;	//Need to know when mech is done so Karnov can fly away.

				if (!dustCloud[commanderID] && g_userPreferences.useNonWeaponEffects)
				{
					if (strcmp(weaponEffects->GetEffectName(KARNOV_DUST_CLOUD),"NONE") != 0)
					{
						//--------------------------------------------
						// Yes, load it on up.
						unsigned flags = 0;

						Check_Object(gosFX::EffectLibrary::Instance);
						gosFX::Effect::Specification* gosEffectSpec = gosFX::EffectLibrary::Instance->Find(weaponEffects->GetEffectName(KARNOV_DUST_CLOUD));

						if (gosEffectSpec)
						{
							dustCloud[commanderID] = gosFX::EffectLibrary::Instance->MakeEffect(gosEffectSpec->m_effectID, flags);
							gosASSERT(dustCloud[commanderID] != NULL);

							MidLevelRenderer::MLRTexturePool::Instance->LoadImages();
						}
					}
				}

				if (!recoveryBeam[commanderID])
				{
					if (strcmp(weaponEffects->GetEffectName(KARNOV_RECOVERY_BEAM),"NONE") != 0)
					{
						//--------------------------------------------
						// Yes, load it on up.
						unsigned flags = 0;

						Check_Object(gosFX::EffectLibrary::Instance);
						gosFX::Effect::Specification* gosEffectSpec = gosFX::EffectLibrary::Instance->Find(weaponEffects->GetEffectName(KARNOV_RECOVERY_BEAM));

						if (gosEffectSpec)
						{
							recoveryBeam[commanderID] = gosFX::EffectLibrary::Instance->MakeEffect(gosEffectSpec->m_effectID, flags);
							gosASSERT(recoveryBeam[commanderID] != NULL);

							MidLevelRenderer::MLRTexturePool::Instance->LoadImages();
						}
					}
				}
			}
		}

		//Should we play another sample to let them know what is coming in after the load?
		//For now, no!
		/*
		switch (vehicleID)
		{
			case 120:	//Minelayer
				soundSystem->playSupportSample(SUPPORT_MINELAYER);
			break;

			case 182:	//Repair
				soundSystem->playSupportSample(SUPPORT_REPAIR);
			break;

			case 221:	//Scout
				soundSystem->playSupportSample(SUPPORT_SCOUT);
			break;

			case 147:	//Recovery
				soundSystem->playSupportSample(SUPPORT_RECOVER);
			break;

			case 415:	//Artillery
				soundSystem->playSupportSample(SUPPORT_RECOVER);
			break;
		}
		*/

		if (vehicleID[commanderID] != 147)
			g_soundSystem->playDigitalSample(VTOL_ANIMATE,vPos[commanderID]);
		else
			g_soundSystem->playDigitalSample(SALVAGE_CRAFT,vPos[commanderID]);

		Stuff::Vector3D newPos = vPos[commanderID];
		float rotation = 0.f;  // this needs to be pointing 180 degrees from drop point

		vTol[commanderID]->setObjectParameters( newPos, 0, false, Team::home->id, 0);
		//eye->update();
		vTol[commanderID]->update();
		vTol[commanderID]->setInView( true );
		vTol[commanderID]->setVisibility( 1, 1 );
		vTol[commanderID]->rotation = rotation;

		if (dustCloud[commanderID])
		{
			Stuff::LinearMatrix4D 	shapeOrigin;
			Stuff::LinearMatrix4D	localToWorld;
			Stuff::LinearMatrix4D	localResult;

			Stuff::Vector3D dustPos = vTol[commanderID]->position;
			Stuff::Point3D wakePos;
			wakePos.x = -dustPos.x;
			wakePos.y = dustPos.z;
			wakePos.z = dustPos.y;

			shapeOrigin.BuildRotation(Stuff::EulerAngles(0.0f,0.0f,0.0f));
			shapeOrigin.BuildTranslation(wakePos);

			gosFX::Effect::ExecuteInfo info((Stuff::Time)g_missionTime,&shapeOrigin,NULL);

			dustCloud[commanderID]->SetLoopOff();
			dustCloud[commanderID]->SetExecuteOn();
			dustCloud[commanderID]->Start(&info);
		}

		paintingVtol[commanderID] = 1;
		vTolSpeed = -75.f;

		vehicleDropped[commanderID] = false;
		mechRecovered[commanderID] = false;

		controlGui.disableAllVehicleButtons();
	}
}

void MissionInterfaceManager::updateRollovers()
{
/*	For all (both LOS and non-LOS) the run cursors, use strings 45149/45150
	For all the jump cursors, use strings 45151/45152
	For all the walk waypoint cursors, use strings 45153/45154
	For all the run waypoint cursors, use strings 45155/45156
	For all the jump waypoint cursors, use strings 45157/45158
	For all the patrol path cursors, use strings 45159/45160
	For all the standard attack cursors, use strings 45161/45162
	For all the ammo conservation cursors, use strings 45163/45164
	For all the fire from current position cursors, use strings 45165/45166
	For all the short-range attack cursors, use strings 45167/45168
	For all the medium-range attack cursors, use strings 45169/45170
	For all the long-range attack cursors, use strings 45171/45172
	For all the force-fire attack cursors, use strings 45173/45174
	For the guard cursor, use strings 45147/45148 (one each for right-click and left-click models).*/

	long cursor = userInput->getMouseCursor();

	if ( ( cursor >= mState_RUN1 && cursor <= mState_RUN1 + 64) )
	{
		helpTextID = 0;
		controlGui.setRolloverHelpText( IDS_RUN_CURSOR_LEFT_HELP );
	}

	else if ( ( cursor >= mState_JUMP1 && cursor <= mState_JUMP1 + 64 ) )
	{
		helpTextID = 0;
		controlGui.setRolloverHelpText( IDS_JUMP_CURSOR_LEFT_HELP );
	}

	else
	{

		switch (cursor)
		{
			case mState_GUARD:
				if ( target )
				{
					helpTextID = 0;
					controlGui.setRolloverHelpText( IDS_GUARD_RECIPIENT_LEFT_HELP );
				}

			break;

			case mState_JUMPWAYPT:
			case mState_JUMPWAYPT_LOS:
				helpTextID = 0;
				controlGui.setRolloverHelpText( IDS_JUMP_WAYPOINT_CURSOR_LEFT_HELP );
				break;
			case mState_RUNWAYPT:
			case mState_RUNWAYPT_LOS:
				helpTextID = 0;
				controlGui.setRolloverHelpText( IDS_RUN_WAYPOINT_CURSOR_LEFT_HELP );

				break;
			case mState_WALKWAYPT:
			case mState_WALKWAYPT_LOS:
				helpTextID = 0;
				controlGui.setRolloverHelpText( IDS_WALK_WAYPOINT_CURSOR_LEFT_HELP );

			break;

			case mState_LINK:
				helpTextID = 0;
				controlGui.setRolloverHelpText( IDS_PATROL_CURSOR_LEFT_HELP );
				break;

			case mState_GENERIC_ATTACK:
			case mState_ATTACK_LOS:
				helpTextID = 0;
				controlGui.setRolloverHelpText( IDS_ATTACK_CURSOR_LEFT_HELP );
				break;

			case mState_ENERGY_WEAPONS:
			case mState_ENERGY_WEAPONS_LOS:
				helpTextID = 0;
				controlGui.setRolloverHelpText( IDS_AMMO_CONSERVE_CURSOR_LEFT_HELP );
				break;

			case mState_CURPOS_ATTACK:
			case mState_CURPOS_ATTACK_LOS:
				helpTextID = 0;
				if ( gos_GetKeyStatus( (gosEnum_KeyIndex)s_commands[FORCE_FIRE_KEY].key ) == KEY_HELD )
					controlGui.setRolloverHelpText(  IDS_FORCE_FIRE_CURSOR_LEFT_HELP );
				else
					controlGui.setRolloverHelpText( IDS_FIRE_FROM_CURRENT_POSITION_CURSOR_LEFT_HELP );
				break;

			case mState_SHRTRNG_ATTACK:
			case mState_SHRTRNG_LOS:
				helpTextID = 0;
				controlGui.setRolloverHelpText( IDS_SHORT_RANGE_CURSOR_LEFT_HELP );
				break;

			case mState_MEDRNG_ATTACK:
			case mState_MEDRNG_LOS:
				helpTextID = 0;
				controlGui.setRolloverHelpText( IDS_MEDIUM_RANGE_CURSOR_LEFT_HELP );
				break;
			
			case mState_LONGRNG_ATTACK:
			case mState_LONGRNG_LOS:
				helpTextID = 0;
				controlGui.setRolloverHelpText( IDS_LONG_RANGE_CURSOR_LEFT_HELP );
				break;
		}
	}
}

//--------------------------------------------------------------------------------------
