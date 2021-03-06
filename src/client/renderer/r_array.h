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

#ifndef __R_ARRAY_H__
#define __R_ARRAY_H__

#include "r_types.h"

#define R_ARRAY_VERTEX			0x1
#define R_ARRAY_COLOR			0x2
#define R_ARRAY_NORMAL			0x4
#define R_ARRAY_TANGENT			0x8
#define R_ARRAY_TEX_DIFFUSE		0x10
#define R_ARRAY_TEX_LIGHTMAP	0x20

#ifdef __R_LOCAL_H__
void R_SetArrayState(const r_model_t *mod);
void R_ResetArrayState(void);
#endif /* __R_LOCAL_H__ */

#endif /* __R_ARRAY_H__ */
