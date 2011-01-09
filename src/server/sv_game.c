/*
 * Copyright(c) 1997-2001 Id Software, Inc.
 * Copyright(c) 2002 The Quakeforge Project.
 * Copyright(c) 2006 Quake2World.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*
 * interface to the game dll
 */

#include "server.h"

game_export_t *ge;


/*
 * Sv_Print
 */
static void Sv_Print(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
static void Sv_Print(const char *fmt, ...){
	char msg[1024];
	va_list	argptr;

	va_start(argptr, fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);

	Com_Print("%s", msg);
}


/*
 * Sv_Debug
 */
static void Sv_Debug(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
static void Sv_Debug(const char *fmt, ...){
	char msg[1024];
	va_list	argptr;

	va_start(argptr, fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);

	Com_Debug("%s", msg);
}


/*
 * Sv_Error
 *
 * Abort the server with a game error
 */
static void Sv_Error(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
static void Sv_Error(const char *fmt, ...){
	char msg[1024];
	va_list	argptr;

	va_start(argptr, fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);

	Com_Error(ERR_DROP, "Game error: %s.\n", msg);
}


/*
 * Sv_SetModel
 *
 * Also sets mins and maxs for inline bmodels
 */
static void Sv_SetModel(edict_t *ent, const char *name){
	int i;
	cmodel_t *mod;

	if(!name){
		Com_Warn("Sv_SetModel %d: NULL.\n", (int)NUM_FOR_EDICT(ent));
		return;
	}

	i = Sv_ModelIndex(name);

	//	ent->model = name;
	ent->s.modelindex = i;

	// if it is an inline model, get the size information for it
	if(name[0] == '*'){
		mod = Cm_InlineModel(name);
		VectorCopy(mod->mins, ent->mins);
		VectorCopy(mod->maxs, ent->maxs);
		Sv_LinkEdict(ent);
	}
}


/*
 * Sv_Configstring
 *
 */
static void Sv_Configstring(int index, const char *val){

	if(index < 0 || index >= MAX_CONFIGSTRINGS){
		Com_Warn("Sv_Configstring: bad index %i.\n", index);
		return;
	}

	if(!val)
		val = "";

	// change the string in sv.configstrings
	strncpy(sv.configstrings[index], val, sizeof(sv.configstrings[0]));

	if(sv.state != ss_loading){  // send the update to everyone
		Sb_Clear(&sv.multicast);
		Msg_WriteChar(&sv.multicast, svc_configstring);
		Msg_WriteShort(&sv.multicast, index);
		Msg_WriteString(&sv.multicast, val);

		Sv_Multicast(vec3_origin, MULTICAST_ALL_R);
	}
}


/*
 * Message wrappers which target the multicast buffer.
 */

static void Sv_WriteChar(int c){
	Msg_WriteChar(&sv.multicast, c);
}

static void Sv_WriteByte(int c){
	Msg_WriteByte(&sv.multicast, c);
}

static void Sv_WriteShort(int c){
	Msg_WriteShort(&sv.multicast, c);
}

static void Sv_WriteLong(int c){
	Msg_WriteLong(&sv.multicast, c);
}

static void Sv_WriteFloat(float f){
	Msg_WriteFloat(&sv.multicast, f);
}

static void Sv_WriteString(const char *s){
	Msg_WriteString(&sv.multicast, s);
}

static void Sv_WritePos(vec3_t pos){
	Msg_WritePos(&sv.multicast, pos);
}

static void Sv_WriteDir(vec3_t dir){
	Msg_WriteDir(&sv.multicast, dir);
}

static void Sv_WriteAngle(float f){
	Msg_WriteAngle(&sv.multicast, f);
}


/*
 * Sv_inPVS
 *
 * Also checks portalareas so that doors block sight
 */
static qboolean Sv_inPVS(const vec3_t p1, const vec3_t p2){
	int leafnum;
	int cluster;
	int area1, area2;
	byte *mask;

	leafnum = Cm_PointLeafnum(p1);
	cluster = Cm_LeafCluster(leafnum);
	area1 = Cm_LeafArea(leafnum);
	mask = Cm_ClusterPVS(cluster);

	leafnum = Cm_PointLeafnum(p2);
	cluster = Cm_LeafCluster(leafnum);
	area2 = Cm_LeafArea(leafnum);

	if(mask &&(!(mask[cluster >> 3] &(1 <<(cluster&7)))))
		return false;

	if(!Cm_AreasConnected(area1, area2))
		return false;  // a door blocks sight

	return true;
}


/*
 * Sv_inPHS
 *
 * Also checks portalareas so that doors block sound
 */
static qboolean Sv_inPHS(const vec3_t p1, const vec3_t p2){
	int leafnum;
	int cluster;
	int area1, area2;
	byte *mask;

	leafnum = Cm_PointLeafnum(p1);
	cluster = Cm_LeafCluster(leafnum);
	area1 = Cm_LeafArea(leafnum);
	mask = Cm_ClusterPHS(cluster);

	leafnum = Cm_PointLeafnum(p2);
	cluster = Cm_LeafCluster(leafnum);
	area2 = Cm_LeafArea(leafnum);
	if(mask &&(!(mask[cluster >> 3] &(1 <<(cluster&7)))))
		return false;  // more than one bounce away
	if(!Cm_AreasConnected(area1, area2))
		return false;  // a door blocks hearing

	return true;
}


/*
 * Sv_StartSound_
 */
static void Sv_Sound(edict_t *ent, int soundindex, int atten){

	if(!ent)
		return;

	Sv_PositionedSound(NULL, ent, soundindex, atten);
}


/*
 * Sv_ShutdownGameProgs
 *
 * Called when either the entire server is being killed, or
 * it is changing to a different game directory.
 */
void Sv_ShutdownGameProgs(void){

	if(!ge)
		return;

	ge->Shutdown();
	ge = NULL;

	Sys_UnloadGame();
}


/*
 * Sv_InitGameProgs
 *
 * Init the game subsystem for a new map
 */
void Sv_InitGameProgs(void){
	game_import_t import;

	if(ge){
		Sv_ShutdownGameProgs();
	}

	import.serverrate = svs.packetrate;
	import.serverframe = 1.0 / svs.packetrate;

	import.Print = Sv_Print;
	import.Debug = Sv_Debug;
	import.BroadcastPrint = Sv_BroadcastPrint;
	import.ClientPrint = Sv_ClientPrint;
	import.ClientCenterPrint = Sv_ClientCenterPrint;

	import.Error = Sv_Error;

	import.Configstring = Sv_Configstring;

	import.ModelIndex = Sv_ModelIndex;
	import.SoundIndex = Sv_SoundIndex;
	import.ImageIndex = Sv_ImageIndex;

	import.SetModel = Sv_SetModel;
	import.Sound = Sv_Sound;
	import.PositionedSound = Sv_PositionedSound;

	import.Trace = Sv_Trace;
	import.PointContents = Sv_PointContents;
	import.inPVS = Sv_inPVS;
	import.inPHS = Sv_inPHS;
	import.SetAreaPortalState = Cm_SetAreaPortalState;
	import.AreasConnected = Cm_AreasConnected;
	import.Pmove = Pmove;

	import.LinkEntity = Sv_LinkEdict;
	import.UnlinkEntity = Sv_UnlinkEdict;
	import.BoxEdicts = Sv_AreaEdicts;

	import.Multicast = Sv_Multicast;
	import.Unicast = Sv_Unicast;
	import.WriteChar = Sv_WriteChar;
	import.WriteByte = Sv_WriteByte;
	import.WriteShort = Sv_WriteShort;
	import.WriteLong = Sv_WriteLong;
	import.WriteFloat = Sv_WriteFloat;
	import.WriteString = Sv_WriteString;
	import.WritePosition = Sv_WritePos;
	import.WriteDir = Sv_WriteDir;
	import.WriteAngle = Sv_WriteAngle;

	import.TagMalloc = Z_TagMalloc;
	import.TagFree = Z_Free;
	import.FreeTags = Z_FreeTags;

	import.Gamedir = Fs_Gamedir;
	import.OpenFile = Fs_OpenFile;
	import.CloseFile = Fs_CloseFile;
	import.LoadFile = Fs_LoadFile;

	import.Cvar = Cvar_Get;

	import.Argc = Cmd_Argc;
	import.Argv = Cmd_Argv;
	import.Args = Cmd_Args;

	import.AddCommandString = Cbuf_AddText;

	ge = (game_export_t *)Sys_LoadGame(&import);

	if(!ge){
		Com_Error(ERR_DROP, "Sv_InitGameProgs: Failed to load game module.\n");
	}
	if(ge->apiversion != GAME_API_VERSION){
		Com_Error(ERR_DROP, "Sv_InitGameProgs: Game is version %i, not %i.\n",
				ge->apiversion, GAME_API_VERSION);
	}

	ge->Init();
}
