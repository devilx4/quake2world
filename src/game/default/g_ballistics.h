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

#ifndef __GAME_BALLISTICS_H__
#define __GAME_BALLISTICS_H__

#include "g_types.h"

#ifdef __GAME_LOCAL_H__
void G_BlasterProjectile(g_edict_t *ent, vec3_t start, vec3_t dir,
		int32_t speed, int32_t damage, int32_t knockback);
void G_BulletProjectile(g_edict_t *ent, vec3_t start, vec3_t dir,
		int32_t damage, int32_t knockback, int32_t hspread, int32_t vspread, int32_t mod);
void G_ShotgunProjectiles(g_edict_t *ent, vec3_t start, vec3_t dir,
		int32_t damage, int32_t knockback, int32_t hspread, int32_t vspread, int32_t count, int32_t mod);
void G_HyperblasterProjectile(g_edict_t *ent, vec3_t start, vec3_t dir,
		int32_t speed, int32_t damage, int32_t knockback);
void G_GrenadeProjectile(g_edict_t *ent, vec3_t start, vec3_t dir,
		int32_t speed, int32_t damage, int32_t knockback, vec_t damage_radius, uint32_t timer);
void G_RocketProjectile(g_edict_t *ent, vec3_t start, vec3_t dir,
		int32_t speed, int32_t damage, int32_t knockback, vec_t damage_radius);
void G_LightningProjectile(g_edict_t *ent, vec3_t start, vec3_t dir,
		int32_t damage, int32_t knockback);
void G_RailgunProjectile(g_edict_t *ent, vec3_t start, vec3_t dir,
		int32_t damage, int32_t knockback);
void G_BfgProjectiles(g_edict_t *ent, vec3_t start, vec3_t dir,
		int32_t speed, int32_t damage, int32_t knockback, vec_t damage_radius);
#endif /* __GAME_LOCAL_H__ */

#endif /* __GAME_BALLISTICS_H__ */
