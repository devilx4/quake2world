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

#include "cl_local.h"

static void *cgame_handle;

/*
 * @brief Abort the server with a game error. This wraps Com_Error, always
 * emitting ERR_DROP.
 */
static void Cl_CgameError(const char *func, const char *fmt, ...) __attribute__((noreturn, format(printf, 2, 3)));
static void Cl_CgameError(const char *func, const char *fmt, ...) {
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

	Com_Error(ERR_DROP, "!Client game error: %s\n", msg);
}

/*
 * Message parsing facilities.
 */

static void Cl_ReadData(void *data, size_t len) {
	Msg_ReadData(&net_message, data, len);
}

static int32_t Cl_ReadChar(void) {
	return Msg_ReadChar(&net_message);
}

static int32_t Cl_ReadByte(void) {
	return Msg_ReadByte(&net_message);
}

static int32_t Cl_ReadShort(void) {
	return Msg_ReadShort(&net_message);
}

static int32_t Cl_ReadLong(void) {
	return Msg_ReadLong(&net_message);
}

static char *Cl_ReadString(void) {
	return Msg_ReadString(&net_message);
}

static void Cl_ReadPosition(vec3_t pos) {
	Msg_ReadPos(&net_message, pos);
}

static void Cl_ReadDirection(vec3_t dir) {
	Msg_ReadDir(&net_message, dir);
}

static vec_t Cl_ReadAngle(void) {
	return Msg_ReadAngle(&net_message);
}

/*
 * @brief
 */
static char *Cl_ConfigString(uint16_t index) {

	if (index > MAX_CONFIG_STRINGS) {
		Com_Warn("Bad index %i\n", index);
		return "";
	}

	return cl.config_strings[index];
}

/*
 * @brief Initializes the client game subsystem
 */
void Cl_InitCgame(void) {
	cg_import_t import;

	if (cls.cgame) {
		Cl_ShutdownCgame();
	}

	Com_Print("Client initialization...\n");

	memset(&import, 0, sizeof(import));

	import.Print = Com_Print;
	import.Debug_ = Com_Debug_;
	import.Warn_ = Com_Warn_;
	import.Error_ = Cl_CgameError;

	import.Malloc = Z_TagMalloc;
	import.LinkMalloc = Z_LinkMalloc;
	import.Free = Z_Free;
	import.FreeTag = Z_FreeTag;

	import.LoadFile = Fs_Load;
	import.FreeFile = Fs_Free;

	import.Cvar = Cvar_Get;
	import.Cmd = Cmd_Add;

	import.ConfigString = Cl_ConfigString;

	import.ReadData = Cl_ReadData;
	import.ReadChar = Cl_ReadChar;
	import.ReadByte = Cl_ReadByte;
	import.ReadShort = Cl_ReadShort;
	import.ReadLong = Cl_ReadLong;
	import.ReadString = Cl_ReadString;
	import.ReadPosition = Cl_ReadPosition;
	import.ReadDirection = Cl_ReadDirection;
	import.ReadAngle = Cl_ReadAngle;

	import.client = &cl;

	import.EntityString = Cm_EntityString;

	import.PointContents = R_PointContents;
	import.Trace = R_Trace;

	import.LeafForPoint = R_LeafForPoint;
	import.LeafInPhs = R_LeafInPhs;
	import.LeafInPvs = R_LeafInPvs;

	import.LoadSample = S_LoadSample;
	import.PlaySample = S_PlaySample;
	import.LoopSample = S_LoopSample;

	import.context = &r_context;

	import.view = &r_view;

	import.palette = palette;
	import.ColorFromPalette = Img_ColorFromPalette;

	import.Color = R_Color;

	import.LoadImage = R_LoadImage;
	import.LoadMaterial = R_LoadMaterial;
	import.LoadModel = R_LoadModel;
	import.WorldModel = R_WorldModel;

	import.AddCorona = R_AddCorona;
	import.AddEntity = R_AddEntity;
	import.AddLinkedEntity = R_AddLinkedEntity;
	import.AddLight = R_AddLight;
	import.AddParticle = R_AddParticle;
	import.AddSustainedLight = R_AddSustainedLight;

	import.DrawImage = R_DrawImage;
	import.DrawFill = R_DrawFill;

	import.BindFont = R_BindFont;
	import.StringWidth = R_StringWidth;
	import.DrawString = R_DrawString;

	cls.cgame = Sys_LoadLibrary("cgame", &cgame_handle, "Cg_LoadCgame", &import);

	if (!cls.cgame) {
		Com_Error(ERR_DROP, "Failed to load client game\n");
	}

	if (cls.cgame->api_version != CGAME_API_VERSION) {
		Com_Error(ERR_DROP, "Client game has wrong version (%d)\n", cls.cgame->api_version);
	}

	cls.cgame->Init();

	Com_Print("Client initialized\n");
	Com_InitSubsystem(Q2W_CGAME);
}

/*
 * @brief Shuts down the client game subsystem.
 */
void Cl_ShutdownCgame(void) {

	if (!cls.cgame)
		return;

	Com_Print("Client game shutdown...\n");

	cls.cgame->Shutdown();
	cls.cgame = NULL;

	Cmd_RemoveAll(CMD_CGAME);

	Z_FreeTag(Z_TAG_CGAME_LEVEL);
	Z_FreeTag(Z_TAG_CGAME);

	Com_Print("Client game down\n");
	Com_QuitSubsystem(Q2W_CGAME);

	Sys_CloseLibrary(&cgame_handle);
}
