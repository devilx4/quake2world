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

#ifndef G_CLIENT_H_
#define G_CLIENT_H_

#ifdef __G_LOCAL_H__

void G_ClientBegin(g_edict_t *ent);
void G_ClientBeginFrame(g_edict_t *ent);
boolean_t G_ClientConnect(g_edict_t *ent, char *user_info);
void G_ClientDisconnect(g_edict_t *ent);
void G_ClientRespawn(g_edict_t *ent, boolean_t voluntary);
void G_ClientThink(g_edict_t *ent, user_cmd_t *ucmd);
void G_ClientUserInfoChanged(g_edict_t *ent, const char *user_info);

#endif /* __G_LOCAL_H__ */

#endif /* G_CLIENT_H_ */