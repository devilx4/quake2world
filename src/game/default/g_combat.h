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

#ifndef __GAME_COMBAT_H__
#define __GAME_COMBAT_H__

#include "g_types.h"

#ifdef __GAME_LOCAL_H__
_Bool G_CanDamage(g_edict_t *targ, g_edict_t *inflictor);
void G_Damage(g_edict_t *targ, g_edict_t *inflictor, g_edict_t *attacker, vec3_t dir,
		vec3_t point, vec3_t normal, int16_t damage, int16_t knockback, int32_t dflags, int32_t mod);
_Bool G_OnSameTeam(const g_edict_t *ent1, const g_edict_t *ent2);
void G_RadiusDamage(g_edict_t *inflictor, g_edict_t *attacker, g_edict_t *ignore,
		int32_t damage, int32_t knockback, vec_t radius, int32_t mod);
#endif /* __GAME_LOCAL_H__ */

#endif /* G_COMBAT_H_ */
