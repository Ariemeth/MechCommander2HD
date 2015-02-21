//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//----------------------------------------------------------------------------------
// Include Files
#include "abl.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

//*****************************************************************************

void* ablSystemMallocCallback (unsigned long memSize) {

	char* mem = new char[memSize];
	return(mem);
}

//-----------------------------------------------------------------------------

void* ablStackMallocCallback (unsigned long memSize) {

	char* mem = new char[memSize];
	return(mem);
}

//-----------------------------------------------------------------------------

void* ablCodeMallocCallback (unsigned long memSize) {

	char* mem = new char[memSize];
	return(mem);
}

//-----------------------------------------------------------------------------

void* ablSymbolMallocCallback (unsigned long memSize) {

	char* mem = new char[memSize];
	return(mem);
}

//-----------------------------------------------------------------------------

void ablSystemFreeCallback (void* memBlock) {

	delete memBlock;
}

//-----------------------------------------------------------------------------

void ablStackFreeCallback (void* memBlock) {

	delete memBlock;
}

//-----------------------------------------------------------------------------

void ablCodeFreeCallback (void* memBlock) {

	delete memBlock;
}

//-----------------------------------------------------------------------------

void ablSymbolFreeCallback (void* memBlock) {

	delete memBlock;
}

//*****************************************************************************

long ablFileCreateCB (void** file, char* fName) {
	(void)file;
	(void)fName;
/*	*file = fopen(fNamenew File;
	if (*file == NULL)
		Fatal(0, " unable to create ABL file");
	if (((FilePtr)*file)->create(fName) != NO_ERR) {
		char s[256];
		sprintf(s, " ABL.ablFileOpenCB: unable to create file [%s] ", fName);
		Fatal(0, s);
	}
*/	return(0);
}

//-----------------------------------------------------------------------------

long ablFileOpenCB (void** file, char* fName) {

	//Filenames MUST be all lowercase or Hash won't find 'em!
	for (size_t i=0;i<strlen(fName);i++)
		fName[i] = (char)tolower(fName[i]);
	*file = fopen(fName, "r");
	if (*file == NULL) {
		char errStr[256];
		sprintf(errStr, "Cannot open \"%s\"", fName);
		printf(errStr);
		scanf("\nPress any key to exit\n");
		exit(0);
	}
	return(0);
}

//-----------------------------------------------------------------------------

long ablFileCloseCB (void** file) {

	fclose((FILE*)file);
	return(0);
}

//-----------------------------------------------------------------------------

bool ablFileEofCB (void* file) {

	int res = feof((FILE*)file);
	return (res != 0);
}

//-----------------------------------------------------------------------------

long ablFileReadCB (void* file, unsigned char* buffer, long length) {

	return(fread(buffer, 1, length, (FILE*)file));
}

//-----------------------------------------------------------------------------

long ablFileReadLongCB (void* file) {

	long value;
	fread(&value, 4, 1, (FILE*)file);
	return(value);
}

//-----------------------------------------------------------------------------

long ablFileReadStringCB (void* file, unsigned char* buffer) {

	buffer[0] = NULL;
	char* s = fgets((char*)buffer, 9999, (FILE*)file);
	if (!s)
		return(strlen((char*)buffer));
	return(strlen(s));
}

//-----------------------------------------------------------------------------

long ablFileReadLineExCB (void* file, unsigned char* buffer, long maxLength) {

	buffer[0] = NULL;
	char* s = fgets((char*)buffer, maxLength, (FILE*)file);
	if (!s)
		return(strlen((char*)buffer));
	return(strlen(s));
}

//-----------------------------------------------------------------------------

long ablFileWriteCB (void* file, unsigned char* buffer, long length) {
	(void)file;
	(void)buffer;
	(void)length;
	//return(((FilePtr)file)->write(buffer, length));
	return(0);
}

//-----------------------------------------------------------------------------

long ablFileWriteByteCB (void* file, unsigned char byte) {
	(void)file;
	(void)byte;
	//return(((FilePtr)file)->writeByte(byte));
	return(0);
}

//-----------------------------------------------------------------------------

long ablFileWriteLongCB (void* file, long value) {
	(void)file;
	(void)value;
	//return(((FilePtr)file)->writeLong(value));
	return(0);
}

//-----------------------------------------------------------------------------

long ablFileWriteStringCB (void* file, char* buffer) {
	(void)file;
	(void)buffer;
	//return(((FilePtr)file)->writeString(buffer));
	return(0);
}

//*****************************************************************************

void ablDebuggerPrintCallback (char* s) {
	(void)s;
	//ABLDebuggerOut->print(s);
//	char msg[1024];
//	sprintf(msg, "%s\n", s);
//	SPEW((0,msg));
}

//*****************************************************************************

void ablDebugPrintCallback (char* s) {
	(void)s;
//	DEBUGWINS_print(s, 0);
}

//*****************************************************************************

void ablSeedRandom (unsigned long seed) {
	(void)seed;
//	gos_srand(seed);
}

//*****************************************************************************

void ablFatalCallback (long code, char* s) {

	printf("\n");
	printf(" FATAL: (%ld) %s", code, s);
	scanf("Press Any Key...");
	exit(code);
}

//*****************************************************************************

void initABL (void) {

	ABLi_init(20479, //AblRunTimeStackSize,
			  51199, //AblMaxCodeBlockSize,
			  200, //AblMaxRegisteredModules,
			  100, //AblMaxStaticVariables,
			  ablSystemMallocCallback,
			  ablStackMallocCallback,
			  ablCodeMallocCallback,
			  ablSymbolMallocCallback,
			  ablSystemFreeCallback,
			  ablStackFreeCallback,
			  ablCodeFreeCallback,
			  ablSymbolFreeCallback,
			  ablFileCreateCB,
			  ablFileOpenCB,
			  ablFileCloseCB,
			  ablFileEofCB,
			  ablFileReadCB,
			  ablFileReadLongCB,
			  ablFileReadStringCB,
			  ablFileReadLineExCB,
			  ablFileWriteCB,
			  ablFileWriteByteCB,
			  ablFileWriteLongCB,
			  ablFileWriteStringCB,
			  ablDebuggerPrintCallback,
			  ablFatalCallback,
			  true,
			  false);

	ABLi_setDebugPrintCallback(ablDebugPrintCallback);

	ABLi_addFunction("getid", false, NULL, "i", NULL);
	ABLi_addFunction("gettime", false, NULL, "r", NULL);
	ABLi_addFunction("gettimeleft", false, NULL, "r", NULL);
	ABLi_addFunction("selectobject", false, "i", "i", NULL);
	//ABLi_addFunction("selectunit", false, "i", "i", NULL);
	ABLi_addFunction("selectwarrior", false, "i", "i", NULL);
	ABLi_addFunction("getwarriorstatus", false, "i", "i", NULL);
	ABLi_addFunction("getcontacts", false, "Iii", "i", NULL);
	ABLi_addFunction("getenemycount", false, "i", "i", NULL);
	ABLi_addFunction("selectcontact", false, "ii", "i", NULL);
	ABLi_addFunction("getcontactid", false, NULL, "i", NULL);
	ABLi_addFunction("iscontact", false, "iii", "i", NULL);
	ABLi_addFunction("getcontactstatus", false, "I", "i", NULL);
	ABLi_addFunction("getcontactrelativeposition", false, "rr", "i", NULL);
	ABLi_addFunction("settarget", false, "ii", NULL, NULL);
	ABLi_addFunction("gettarget", false, "i", "i", NULL);
	ABLi_addFunction("getweaponsready", false, "Ii", "i", NULL);
	ABLi_addFunction("getweaponslocked", false, "Ii", "i", NULL);
	ABLi_addFunction("getweaponsinrange", false, "Ii", "i", NULL);
	ABLi_addFunction("getweaponshots", false, "i", "i", NULL);
	ABLi_addFunction("getweaponranges", false, "iR", NULL, NULL);
	ABLi_addFunction("getobjectposition", false, "iR", NULL, NULL);
	ABLi_addFunction("getintegermemory", false, "i", "i", NULL);
	ABLi_addFunction("getrealmemory", false, "i", "r", NULL);
	ABLi_addFunction("getalarmtriggers", false, "I", "i", NULL);
	ABLi_addFunction("getchallenger", false, "i", "i", NULL);
	ABLi_addFunction("gettimewithoutorders", false, NULL, "r", NULL);
	ABLi_addFunction("getfireranges", false, "R", NULL, NULL);
	ABLi_addFunction("getattackers", false, "Ir", "i", NULL);
	ABLi_addFunction("getattackerinfo", false, "i", "r", NULL);
	ABLi_addFunction("setchallenger", false, "ii", "i", NULL);
	ABLi_addFunction("setintegermemory", false, "ii", NULL, NULL);
	ABLi_addFunction("setrealmemory", false, "ir", NULL, NULL);
	ABLi_addFunction("hasmovegoal", false, NULL, "b", NULL);
	ABLi_addFunction("hasmovepath", false, NULL, "b", NULL);
	ABLi_addFunction("sortweapons", false, "Ii", "i", NULL);
	ABLi_addFunction("getvisualrange", false, "i", "r", NULL);
	ABLi_addFunction("getunitmates", false, "iI", "i", NULL);
	ABLi_addFunction("gettacorder", false, "irI", "i", NULL);
	ABLi_addFunction("getlasttacorder", false, "irI", "i", NULL);
	ABLi_addFunction("getobjects", false, "iI", "i", NULL);
	ABLi_addFunction("orderwait", false, "rb", "i", NULL);
	ABLi_addFunction("ordermoveto", false, "Rb", "i", NULL);
	ABLi_addFunction("ordermovetoobject", false, "ib", "i", NULL);
	ABLi_addFunction("ordermovetocontact", false, "b", "i", NULL);
	ABLi_addFunction("orderpowerdown", false, NULL, "i", NULL);
	ABLi_addFunction("orderpowerup", false, NULL, "i", NULL);
	ABLi_addFunction("orderattackobject", false, "iiiib", "i", NULL);
	ABLi_addFunction("orderattackcontact", false, "iiib", "i", NULL);
	ABLi_addFunction("orderwithdraw", false, NULL, "i", NULL);
	ABLi_addFunction("objectinwithdrawal", false, "i", "i", NULL);
	ABLi_addFunction("damageobject", false, "iiirirr", "i", NULL);
	ABLi_addFunction("setattackradius", false, "r", "r", NULL);
	ABLi_addFunction("objectchangesides", false, "ii", NULL, NULL);
	ABLi_addFunction("distancetoobject", false, "ii", "r", NULL);
	ABLi_addFunction("distancetoposition", false, "iR", "r", NULL);
	ABLi_addFunction("objectsuicide", false, "i", NULL, NULL);
	ABLi_addFunction("objectcreate", false, "i", "i", NULL);
	ABLi_addFunction("objectexists", false, "i", "i", NULL);
	ABLi_addFunction("objectstatus", false, "i", "i", NULL);
	ABLi_addFunction("objectstatuscount", false, "iI", NULL, NULL);
	ABLi_addFunction("objectvisible", false, "ii", "i", NULL);
	ABLi_addFunction("objectside", false, "i", "i", NULL);
	ABLi_addFunction("objectcommander", false, "i", "i", NULL);
	ABLi_addFunction("objectclass", false, "i", "i", NULL);
	ABLi_addFunction("settimer", false, "i*", "i", NULL);
	ABLi_addFunction("checktimer", false, "i", "r", NULL);
	ABLi_addFunction("endtimer", false, "i", NULL, NULL);
	ABLi_addFunction("setobjectivetimer", false, "i*", "i", NULL);
	ABLi_addFunction("checkobjectivetimer", false, "i", "r", NULL);
	ABLi_addFunction("setobjectivestatus", false, "ii", "i", NULL);
	ABLi_addFunction("checkobjectivestatus", false, "i", "i", NULL);
	ABLi_addFunction("setobjectivetype", false, "ii", "i", NULL);
	ABLi_addFunction("checkobjectivetype", false, "i", "i", NULL);
	ABLi_addFunction("playdigitalmusic", false, "i", "i", NULL);
	ABLi_addFunction("stopmusic", false, NULL, "i", NULL);
	ABLi_addFunction("playsoundeffect", false, "i", "i", NULL);
	ABLi_addFunction("playvideo", false, "i", "i", NULL);
	ABLi_addFunction("setradio", false, "ib", "i", NULL);
	ABLi_addFunction("playspeech", false, "ii", "i", NULL);
	ABLi_addFunction("playbetty", false, "i", "i", NULL);
	ABLi_addFunction("setobjectactive", false, "ib", "i", NULL);
	ABLi_addFunction("objecttypeid", false, "i", "i", NULL);
	ABLi_addFunction("getterrainobjectpartid", false, "ii", "i", NULL);
	ABLi_addFunction("objectremove", false, "i", "i", NULL);
	ABLi_addFunction("inarea", false, "iRri", "b", NULL);
	ABLi_addFunction("createinfantry", false, "Ri", "i", NULL);
	ABLi_addFunction("getsensorsworking", false, "i", "i", NULL);
	ABLi_addFunction("getcurrentbrvalue", false, "i", "i", NULL);
	ABLi_addFunction("setcurrentbrvalue", false, "ii", NULL, NULL);
	ABLi_addFunction("getarmorpts", false, "i", "i", NULL);
	ABLi_addFunction("getmaxarmor", false, "i", "i", NULL);
	ABLi_addFunction("getpilotid", false, "i", "i", NULL);
	ABLi_addFunction("getpilotwounds", false, "i", "r", NULL);
	ABLi_addFunction("setpilotwounds", false, "ii", NULL, NULL);
	ABLi_addFunction("getobjectactive", false, "i", "i", NULL);
	ABLi_addFunction("getobjectdamage", false, "i", "i", NULL);
	ABLi_addFunction("getobjectdmgpts", false, "i", "i", NULL);
	ABLi_addFunction("getobjectmaxdmg", false, "i", "i", NULL);
	ABLi_addFunction("setobjectdamage", false, "i", "i", NULL);
	ABLi_addFunction("getglobalvalue", false, "i", "r", NULL);
	ABLi_addFunction("setglobalvalue", false, "i*", NULL, NULL);
	ABLi_addFunction("setobjectivepos", false, "i***", NULL, NULL);
	ABLi_addFunction("setsensorrange", false, "ir", NULL, NULL);
	ABLi_addFunction("settonnage", false, "ir", NULL, NULL);
	ABLi_addFunction("setexplosiondamage", false, "ir", NULL, NULL);
	ABLi_addFunction("setexplosionradius", false, "ir", NULL, NULL);
	ABLi_addFunction("setsalvage", false, "iii", "b", NULL);
	ABLi_addFunction("setsalvagestatus", false, "ib", "b", NULL);
	ABLi_addFunction("setanimation", false, "iii", NULL, NULL);
	ABLi_addFunction("setrevealed", false, "i*R", NULL, NULL);
	ABLi_addFunction("getsalvage", false, "iiII", NULL, NULL);
	ABLi_addFunction("orderrefit", false, "ii", NULL, NULL);
	ABLi_addFunction("setcaptured", false, "i", NULL, NULL);
	ABLi_addFunction("ordercapture", false, "ii", NULL, NULL);
	ABLi_addFunction("setcapturable", false, "ib", NULL, NULL);
	ABLi_addFunction("iscaptured", false, "i", "i", NULL);
	ABLi_addFunction("iscapturable", false, "ii", "b", NULL);
	ABLi_addFunction("wasevercapturable", false, "i", "b", NULL);
	ABLi_addFunction("setbuildingname", false, "ii", NULL, NULL);
	ABLi_addFunction("callstrike", false, "iirrrb", NULL, NULL);
	ABLi_addFunction("callstrikeex", false, "iirrrbr", NULL, NULL);
	ABLi_addFunction("orderloadelementals", false, "i", NULL, NULL);
	ABLi_addFunction("orderdeployelementals", false, "i", NULL, NULL);
	ABLi_addFunction("addprisoner", false, "ii", "i", NULL);
	ABLi_addFunction("lockgateopen", false, "i", NULL, NULL);
	ABLi_addFunction("lockgateclosed", false, "i", NULL, NULL);
	ABLi_addFunction("releasegatelock", false, "i", NULL, NULL);
	ABLi_addFunction("isgateopen", false, "i", "b", NULL);
	ABLi_addFunction("getrelativepositiontopoint", false, "RrriR", NULL, NULL);
	ABLi_addFunction("getrelativepositiontoobject", false, "irriR", NULL, NULL);
	ABLi_addFunction("getunitstatus", false, "i", "r", NULL);
	ABLi_addFunction("repair", false, "ir", NULL, NULL);
	ABLi_addFunction("getfixed", false, "iii", "i", NULL);
	ABLi_addFunction("getrepairstate", false, "i", "i", NULL);
	ABLi_addFunction("isteamtargeting", false, "iii", "b", NULL);
	ABLi_addFunction("isteamcapturing", false, "iii", "b", NULL);
	ABLi_addFunction("sendmessage", false, "ii", NULL, NULL);
	ABLi_addFunction("getmessage", false, "i", "i", NULL);
	ABLi_addFunction("gethometeam", false, NULL, "i", NULL);
	ABLi_addFunction("getstrikes", false, "ii", "i", NULL);
	ABLi_addFunction("setstrikes", false, "iii", NULL, NULL);
	ABLi_addFunction("addstrikes", false, "iii", NULL, NULL);
	ABLi_addFunction("isserver", false, NULL, "b", NULL);
	ABLi_addFunction("calcpartid", false, "iiii", "i", NULL);
	ABLi_addFunction("setdebugstring", false, "iiC", NULL, NULL);
	ABLi_addFunction("break", false, NULL, NULL, NULL);
	ABLi_addFunction("pathexists", false, "iiiii", "i", NULL);
	ABLi_addFunction("convertcoords", false, "iRI", "i", NULL);
	ABLi_addFunction("newmoveto", true, "Ri", "i", NULL);
	ABLi_addFunction("newmovetoobject", true, "ii", "i", NULL);
	ABLi_addFunction("newpower", true, "b", "i", NULL);
	ABLi_addFunction("newattack", true, "ii", "i", NULL);
	ABLi_addFunction("newcapture", true, "ii", "i", NULL);
	ABLi_addFunction("newscan", true, "ii", "i", NULL);
	ABLi_addFunction("newcontrol", true, "ii", "i", NULL);
	ABLi_addFunction("coremoveto", true, "Ri", "i", NULL);
	ABLi_addFunction("coremovetoobject", true, "ii", "i", NULL);
	ABLi_addFunction("corepower", true, "b", "i", NULL);
	ABLi_addFunction("coreattack", true, "ii", "i", NULL);
	ABLi_addFunction("corecapture", true, "ii", "i", NULL);
	ABLi_addFunction("corescan", true, "ii", "i", NULL);
	ABLi_addFunction("corecontrol", true, "ii", "i", NULL);
	ABLi_addFunction("coreeject", true, NULL, "i", NULL);
	ABLi_addFunction("setpilotstate", false, "i", "i", NULL);
	ABLi_addFunction("getpilotstate", false, NULL, "i", NULL);
	ABLi_addFunction("getnextpilotevent", false, "I", "i", NULL);
	ABLi_addFunction("settargetpriority", false, "iiiii", "i", NULL);
	ABLi_addFunction("setdebugwindow", false, "ii", "i", NULL);
	ABLi_addFunction("getcameraposition", false, "R", NULL, NULL);
	ABLi_addFunction("setcameraposition", false, "R", NULL, NULL);
	ABLi_addFunction("setcameragoalposition", false, "Rr", NULL, NULL);
	ABLi_addFunction("getcameragoalposition", false, "R", NULL, NULL);
	ABLi_addFunction("getcamerarotation", false, "R", NULL, NULL);
	ABLi_addFunction("setcamerarotation", false, "R", NULL, NULL);
	ABLi_addFunction("setcameragoalrotation", false, "Rr", NULL, NULL);
	ABLi_addFunction("getcameragoalrotation", false, "R", NULL, NULL);
	ABLi_addFunction("getcamerazoom", false, NULL, "r", NULL);
	ABLi_addFunction("setcamerazoom", false, "r", NULL, NULL);
	ABLi_addFunction("getcameragoalzoom", false, NULL, "r", NULL);
	ABLi_addFunction("setcameragoalzoom", false, "rr", NULL, NULL);
	ABLi_addFunction("setcameravelocity", false, "R", NULL, NULL);
	ABLi_addFunction("getcameravelocity", false, "R", NULL, NULL);
	ABLi_addFunction("setcameragoalvelocity", false, "Rr", NULL, NULL);
	ABLi_addFunction("getcameragoalvelocity", false, "R", NULL, NULL);
	ABLi_addFunction("setcameralookobject", false, "i", NULL, NULL);
	ABLi_addFunction("getcameralookobject", false, NULL, "i", NULL);
	ABLi_addFunction("getcameraframelength", false, NULL, "r", NULL);
	ABLi_addFunction("requesthelp", false, "iRrRri", "r", NULL);
	ABLi_addFunction("requesttarget", false, "Rr", "i", NULL);
	ABLi_addFunction("requestshelter", false, "*", "i", NULL);
	ABLi_addFunction("mcprint", false, "?", NULL, NULL);

	ABLi_addFunction("getmissionstatus", false, NULL, "i", NULL);
	ABLi_addFunction("addtriggerarea", false, "iiiiii", "i", NULL);
	ABLi_addFunction("istriggerareahit", false, "i", "b", NULL);
	ABLi_addFunction("resettriggerarea", false, "i", NULL, NULL);
	ABLi_addFunction("removetriggerarea", false, "i", NULL, NULL);
	ABLi_addFunction("getweapons", false, "Ii", "i", NULL);
	ABLi_addFunction("setmovearea", false, "Rr", NULL, NULL);
	ABLi_addFunction("getweaponsstatus", false, "I", "i", NULL);
	ABLi_addFunction("cleartacorder", false, NULL, NULL, NULL);
	ABLi_addFunction("playwave", false, "Ci", "i", NULL);
	ABLi_addFunction("objectteam", false, "i", "i", NULL);
	ABLi_addFunction("setwillhelp", false, "b", "b", NULL);
	ABLi_addFunction("getlastscan", false, NULL, "i", NULL);
	ABLi_addFunction("getmapinfo", false, "I", NULL, NULL);
}

//*****************************************************************************

void closeABL (void) {

	ABLi_close();
}

//*****************************************************************************

