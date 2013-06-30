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

#include "sv_local.h"
#include "pmove.h"

/*
 * @brief Abort the server with a game error, always emitting ERR_DROP.
 */
static void Sv_GameError(const char *func, const char *fmt, ...) __attribute__((noreturn, format(printf, 2, 3)));
static void Sv_GameError(const char *func, const char *fmt, ...) {
	char msg[MAX_STRING_CHARS];

	if (fmt[0] != '!') {
		g_snprintf(msg, sizeof(msg), "%s: ", func);
	} else {
		msg[0] = '\0';
	}

	const size_t len = strlen(msg);
	va_list args;

	va_start(args, fmt);
	vsnprintf(msg + len, sizeof(msg) - len, fmt, args);
	va_end(args);

	Com_Error(ERR_DROP, "!Game error: %s\n", msg);
}

/*
 * @brief Also sets mins and maxs for inline bsp models.
 */
static void Sv_SetModel(g_edict_t *ent, const char *name) {
	c_model_t *mod;

	if (!name) {
		Com_Warn("%d: NULL\n", (int32_t) NUM_FOR_EDICT(ent));
		return;
	}

	ent->s.model1 = Sv_ModelIndex(name);

	// if it is an inline model, get the size information for it
	if (name[0] == '*') {
		mod = Cm_Model(name);
		VectorCopy(mod->mins, ent->mins);
		VectorCopy(mod->maxs, ent->maxs);
		Sv_LinkEdict(ent);
	}
}

/*
 * @brief
 */
static void Sv_ConfigString(const uint16_t index, const char *val) {

	if (index >= MAX_CONFIG_STRINGS) {
		Com_Warn("Bad index %u\n", index);
		return;
	}

	if (!val)
		val = "";

	// make sure it's actually changed
	if (!g_strcmp0(sv.config_strings[index], val)) {
		return;
	}

	// change the string in sv.config_strings
	g_strlcpy(sv.config_strings[index], val, sizeof(sv.config_strings[0]));

	if (sv.state != SV_LOADING) { // send the update to everyone
		Sb_Clear(&sv.multicast);
		Msg_WriteChar(&sv.multicast, SV_CMD_CONFIG_STRING);
		Msg_WriteShort(&sv.multicast, index);
		Msg_WriteString(&sv.multicast, val);

		Sv_Multicast(vec3_origin, MULTICAST_ALL_R);
	}
}

/*
 * Message wrappers which target the multicast buffer.
 */

static void Sv_WriteData(const void *data, size_t len) {
	Msg_WriteData(&sv.multicast, data, len);
}

static void Sv_WriteChar(const int32_t c) {
	Msg_WriteChar(&sv.multicast, c);
}

static void Sv_WriteByte(const int32_t c) {
	Msg_WriteByte(&sv.multicast, c);
}

static void Sv_WriteShort(const int32_t c) {
	Msg_WriteShort(&sv.multicast, c);
}

static void Sv_WriteLong(const int32_t c) {
	Msg_WriteLong(&sv.multicast, c);
}

static void Sv_WriteString(const char *s) {
	Msg_WriteString(&sv.multicast, s);
}

static void Sv_WritePos(const vec3_t pos) {
	Msg_WritePos(&sv.multicast, pos);
}

static void Sv_WriteDir(const vec3_t dir) {
	Msg_WriteDir(&sv.multicast, dir);
}

static void Sv_WriteAngle(const vec_t v) {
	Msg_WriteAngle(&sv.multicast, v);
}

/*
 * @brief Also checks portal_areas so that doors block sight
 */
static _Bool Sv_InPVS(const vec3_t p1, const vec3_t p2) {
	int32_t leaf_num;
	int32_t cluster;
	int32_t area1, area2;
	byte *mask;

	leaf_num = Cm_PointLeafnum(p1);
	cluster = Cm_LeafCluster(leaf_num);
	area1 = Cm_LeafArea(leaf_num);
	mask = Cm_ClusterPVS(cluster);

	leaf_num = Cm_PointLeafnum(p2);
	cluster = Cm_LeafCluster(leaf_num);
	area2 = Cm_LeafArea(leaf_num);

	if (mask && (!(mask[cluster >> 3] & (1 << (cluster & 7)))))
		return false;

	if (!Cm_AreasConnected(area1, area2))
		return false; // a door blocks sight

	return true;
}

/*
 * @brief Also checks portal_areas so that doors block sound
 */
static _Bool Sv_InPHS(const vec3_t p1, const vec3_t p2) {
	int32_t leaf_num;
	int32_t cluster;
	int32_t area1, area2;
	byte *mask;

	leaf_num = Cm_PointLeafnum(p1);
	cluster = Cm_LeafCluster(leaf_num);
	area1 = Cm_LeafArea(leaf_num);
	mask = Cm_ClusterPHS(cluster);

	leaf_num = Cm_PointLeafnum(p2);
	cluster = Cm_LeafCluster(leaf_num);
	area2 = Cm_LeafArea(leaf_num);

	if (mask && (!(mask[cluster >> 3] & (1 << (cluster & 7)))))
		return false; // more than one bounce away

	if (!Cm_AreasConnected(area1, area2))
		return false; // a door blocks hearing

	return true;
}

/*
 * @brief
 */
static void Sv_Sound(const g_edict_t *ent, const uint16_t index, const uint16_t atten) {

	if (!ent)
		return;

	Sv_PositionedSound(NULL, ent, index, atten);
}

static void *game_handle;

/*
 * @brief Initializes the game module by exposing a subset of server functionality
 * through function pointers. In return, the game module allocates memory for
 * entities and returns a few pointers of its own.
 *
 * Note that the terminology here is worded from the game module's perspective;
 * that is, "import" is what we give to the game, and "export" is what the game
 * returns to us. This distinction seems a bit backwards, but it was likely
 * deemed less confusing to "mod" authors back in the day.
 */
void Sv_InitGame(void) {
	g_import_t import;

	if (svs.game) {
		Sv_ShutdownGame();
	}

	Com_Print("Game initialization...\n");

	memset(&import, 0, sizeof(import));

	import.frame_rate = svs.frame_rate;
	import.frame_millis = 1000 / svs.frame_rate;
	import.frame_seconds = 1.0 / svs.frame_rate;

	import.Print = Com_Print;
	import.Debug_ = Com_Debug_;
	import.Warn_ = Com_Warn_;
	import.Error_ = Sv_GameError;

	import.Malloc = Z_TagMalloc;
	import.LinkMalloc = Z_LinkMalloc;
	import.Free = Z_Free;
	import.FreeTag = Z_FreeTag;

	import.LoadFile = Fs_Load;
	import.FreeFile = Fs_Free;

	import.Cvar = Cvar_Get;
	import.Cmd = Cmd_Add;
	import.Argc = Cmd_Argc;
	import.Argv = Cmd_Argv;
	import.Args = Cmd_Args;

	import.AddCommandString = Cbuf_AddText;

	import.ConfigString = Sv_ConfigString;

	import.ModelIndex = Sv_ModelIndex;
	import.SoundIndex = Sv_SoundIndex;
	import.ImageIndex = Sv_ImageIndex;

	import.SetModel = Sv_SetModel;
	import.Sound = Sv_Sound;
	import.PositionedSound = Sv_PositionedSound;

	import.Trace = Sv_Trace;
	import.PointContents = Sv_PointContents;
	import.inPVS = Sv_InPVS;
	import.inPHS = Sv_InPHS;
	import.SetAreaPortalState = Cm_SetAreaPortalState;
	import.AreasConnected = Cm_AreasConnected;
	import.Pmove = Pmove;

	import.LinkEdict = Sv_LinkEdict;
	import.UnlinkEdict = Sv_UnlinkEdict;
	import.AreaEdicts = Sv_AreaEdicts;

	import.Multicast = Sv_Multicast;
	import.Unicast = Sv_Unicast;
	import.WriteData = Sv_WriteData;
	import.WriteChar = Sv_WriteChar;
	import.WriteByte = Sv_WriteByte;
	import.WriteShort = Sv_WriteShort;
	import.WriteLong = Sv_WriteLong;
	import.WriteString = Sv_WriteString;
	import.WritePosition = Sv_WritePos;
	import.WriteDir = Sv_WriteDir;
	import.WriteAngle = Sv_WriteAngle;

	import.BroadcastPrint = Sv_BroadcastPrint;
	import.ClientPrint = Sv_ClientPrint;

	svs.game = (g_export_t *) Sys_LoadLibrary("game", &game_handle, "G_LoadGame", &import);

	if (!svs.game) {
		Com_Error(ERR_DROP, "Failed to load game module\n");
	}

	if (svs.game->api_version != GAME_API_VERSION) {
		Com_Error(ERR_DROP, "Game is version %i, not %i\n", svs.game->api_version,
				GAME_API_VERSION);
	}

	svs.game->Init();

	Com_Print("Game initialized, starting...\n");
	Com_InitSubsystem(Q2W_GAME);
}

/*
 * @brief Called when either the entire server is being killed, or it is changing to a
 * different game directory.
 */
void Sv_ShutdownGame(void) {

	if (!svs.game)
		return;

	Com_Print("Game shutdown...\n");

	svs.game->Shutdown();
	svs.game = NULL;

	Cmd_RemoveAll(CMD_GAME);

	// the game module code should call this, but lets not assume
	Z_FreeTag(Z_TAG_GAME_LEVEL);
	Z_FreeTag(Z_TAG_GAME);

	Com_Print("Game down\n");
	Com_QuitSubsystem(Q2W_GAME);

	Sys_CloseLibrary(&game_handle);
}
