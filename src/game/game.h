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

#ifndef __GAME_H__
#define __GAME_H__

#include "shared.h"

#define GAME_API_VERSION 1

// edict->sv_flags
#define SVF_NO_CLIENT 1  // don't send entity to clients
// edict->solid values
typedef enum {
	SOLID_NOT, // no interaction with other objects
	SOLID_TRIGGER, // only touch when inside, after moving
	SOLID_BOX, // touch on edge
	SOLID_MISSILE, // touch on edge
	SOLID_BSP
// bsp clip, touch on edge
} solid_t;

// link_t is only used for entity area links now
typedef struct link_s {
	struct link_s *prev, *next;
} link_t;

#define MAX_ENT_CLUSTERS 16

typedef struct g_client_s g_client_t; // typedef'ed here, defined below
typedef struct g_edict_s g_edict_t; // OR in game module

#ifndef __G_LOCAL_H__

/*
 * This is the server's definition of the client and edict structures. The
 * game module is free to add additional members to these structures, provided
 * they communicate the actual size of them at runtime through the game export
 * structure.
 */

struct g_client_s {
	player_state_t ps; // communicated by server to clients
	int ping;
};

struct g_edict_s {
	entity_state_t s;
	struct g_client_s *client;

	bool in_use;
	int link_count;

	// FIXME: move these fields to a server private sv_entity_t
	link_t area; // linked to a division node or leaf

	int num_clusters; // if -1, use head_node instead
	int cluster_nums[MAX_ENT_CLUSTERS];
	int head_node; // unused if num_clusters != -1
	int area_num, area_num2;

	unsigned int sv_flags; // SVF_NO_CLIENT, etc
	vec3_t mins, maxs;
	vec3_t abs_mins, abs_maxs, size;
	solid_t solid;
	unsigned int clip_mask;
	g_edict_t *owner;

// the game can add anything it wants after
// this point in the structure
};

#endif  /* !__G_LOCAL_H__ */

// functions provided by the main engine
typedef struct g_import_s {

	unsigned int frame_rate; // server frames per second
	unsigned int frame_millis;
	float frame_seconds; // seconds per frame

	void (*Print)(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
	void (*Debug)(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
	void (*BroadcastPrint)(const int level, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
	void (*ClientPrint)(const g_edict_t *ent, const int level, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

	void (*Error)(const char *fmt, ...) __attribute__((noreturn, format(printf, 1, 2)));

	// config_strings are used to transmit arbitrary tokens such
	// as model names, skin names, team names, and weather effects
	void (*ConfigString)(const unsigned short index, const char *string);

	// create config_strings and some internal server state
	unsigned short (*ModelIndex)(const char *name);
	unsigned short (*SoundIndex)(const char *name);
	unsigned short (*ImageIndex)(const char *name);

	void (*SetModel)(g_edict_t *ent, const char *name);
	void (*Sound)(const g_edict_t *ent, const unsigned short index, const unsigned short atten);
	void (*PositionedSound)(const vec3_t origin, const g_edict_t *ent, const unsigned short index,
			const unsigned short atten);

	// collision detection
	c_trace_t (*Trace)(const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
			const g_edict_t *passent, const int mask);
	int (*PointContents)(const vec3_t point);
	bool (*inPVS)(const vec3_t p1, const vec3_t p2);
	bool (*inPHS)(const vec3_t p1, const vec3_t p2);
	void (*SetAreaPortalState)(int portal_num, bool open);
	bool (*AreasConnected)(int area1, int area2);
	void (*Pmove)(pm_move_t *pmove); // player movement code common with client prediction

	// an entity will never be sent to a client or used for collision
	// if it is not passed to linkentity.  if the size, position, or
	// solidity changes, it must be relinked.
	void (*LinkEntity)(g_edict_t *ent);
	void (*UnlinkEntity)(g_edict_t *ent); // call before removing an interactive edict
	int (*AreaEdicts)(const vec3_t mins, const vec3_t maxs, g_edict_t **area_edicts,
			const int max_area_edicts, const int area_type);

	// network messaging
	void (*Multicast)(const vec3_t origin, multicast_t to);
	void (*Unicast)(const g_edict_t *ent, const bool reliable);
	void (*WriteData)(const void *data, size_t len);
	void (*WriteChar)(const int c);
	void (*WriteByte)(const int c);
	void (*WriteShort)(const int c);
	void (*WriteLong)(const int c);
	void (*WriteString)(const char *s);
	void (*WritePosition)(const vec3_t pos); // some fractional bits
	void (*WriteDir)(const vec3_t pos); // single byte encoded, very coarse
	void (*WriteAngle)(const float f);

	// managed memory allocation
	void *(*Malloc)(size_t size, short tag);
	void (*Free)(void *ptr);
	void (*FreeTag)(short tag);

	// filesystem interaction
	const char *(*Gamedir)(void);
	int (*OpenFile)(const char *file_name, FILE **file, file_mode_t mode);
	void (*CloseFile)(FILE *file);
	int (*LoadFile)(const char *file_name, void **buffer);

	// console variable interaction
	cvar_t *(*Cvar)(const char *name, const char *value, unsigned int flags, const char *desc);

	// command function parameter access
	int (*Argc)(void);
	char *(*Argv)(int n);
	char *(*Args)(void); // concatenation of all argv >= 1

	// add commands to the server console as if they were typed in
	// for map changing, etc
	void (*AddCommandString)(const char *text);
} g_import_t;

// functions exported by the game subsystem
typedef struct g_export_s {
	int api_version;

	// the init function will only be called when a game starts,
	// not each time a level is loaded.  Persistent data for clients
	// and the server can be allocated in init
	void (*Init)(void);
	void (*Shutdown)(void);

	// each new level entered will cause a call to SpawnEntities
	void (*SpawnEntities)(const char *name, const char *entities);

	bool (*ClientConnect)(g_edict_t *ent, char *user_info);
	void (*ClientBegin)(g_edict_t *ent);
	void (*ClientUserInfoChanged)(g_edict_t *ent, const char *user_info);
	void (*ClientDisconnect)(g_edict_t *ent);
	void (*ClientCommand)(g_edict_t *ent);
	void (*ClientThink)(g_edict_t *ent, user_cmd_t *cmd);

	void (*Frame)(void);

	const char *(*GameName)(void);

	// global variables shared between game and server

	// The edict array is allocated in the game dll so it
	// can vary in size from one game to another.
	//
	// The size will be fixed when ge->Init() is called
	struct g_edict_s *edicts;
	size_t edict_size;
	unsigned short num_edicts; // current number, <= max_edicts
	unsigned short max_edicts;
} g_export_t;

#endif /* __GAME_H__ */
