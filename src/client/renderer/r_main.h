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

#ifndef __R_MAIN_H__
#define __R_MAIN_H__

#include "r_types.h"

// settings and preferences
extern cvar_t *r_anisotropy;
extern cvar_t *r_brightness;
extern cvar_t *r_bumpmap;
extern cvar_t *r_contrast;
extern cvar_t *r_coronas;
extern cvar_t *r_draw_buffer;
extern cvar_t *r_flares;
extern cvar_t *r_fog;
extern cvar_t *r_fullscreen;
extern cvar_t *r_gamma;
extern cvar_t *r_hardness;
extern cvar_t *r_height;
extern cvar_t *r_invert;
extern cvar_t *r_lightmap_block_size;
extern cvar_t *r_lighting;
extern cvar_t *r_line_alpha;
extern cvar_t *r_line_width;
extern cvar_t *r_materials;
extern cvar_t *r_modulate;
extern cvar_t *r_monochrome;
extern cvar_t *r_multisample;
extern cvar_t *r_optimize;
extern cvar_t *r_parallax;
extern cvar_t *r_programs;
extern cvar_t *r_render_mode;
extern cvar_t *r_saturation;
extern cvar_t *r_screenshot_quality;
extern cvar_t *r_shadows;
extern cvar_t *r_specular;
extern cvar_t *r_swap_interval;
extern cvar_t *r_texture_mode;
extern cvar_t *r_vertex_buffers;
extern cvar_t *r_warp;
extern cvar_t *r_width;
extern cvar_t *r_windowed_height;
extern cvar_t *r_windowed_width;

extern r_view_t r_view;

void R_Init(void);
void R_Shutdown(void);
void R_LoadMedia(void);
void R_BeginFrame(void);
void R_DrawView(void);
void R_EndFrame(void);

#if defined(__R_LOCAL_H__) || defined(__ECLIPSE__)

// private hardware configuration information
typedef struct r_config_s {
	const char *renderer_string;
	const char *vendor_string;
	const char *version_string;
	const char *extensions_string;

	int32_t max_texunits;
	int32_t max_teximage_units;
}r_config_t;

extern r_config_t r_config;

// private renderer structure
typedef struct r_locals_s {

	int16_t cluster; // visibility cluster at origin
	int16_t old_cluster;

	byte vis_data_pvs[MAX_BSP_LEAFS >> 3]; // decompressed PVS at origin
	byte vis_data_phs[MAX_BSP_LEAFS >> 3]; // decompressed PHS at origin

	int16_t vis_frame; // PVS frame, negatives are special cases

	int16_t frame; // renderer frame, negatives are special cases
	int16_t back_frame; // back-facing renderer frame

	int16_t light_frame; // dynamic lighting frame

	uint64_t active_light_mask; // a bit mask into r_view.lights
	uint16_t active_light_count; // a count of active lights

	c_bsp_plane_t frustum[4]; // for box culling
}r_locals_t;

extern r_locals_t r_locals;

// development tools
extern cvar_t *r_clear;
extern cvar_t *r_cull;
extern cvar_t *r_lock_vis;
extern cvar_t *r_no_vis;
extern cvar_t *r_draw_bsp_leafs;
extern cvar_t *r_draw_bsp_lightmaps;
extern cvar_t *r_draw_bsp_lights;
extern cvar_t *r_draw_bsp_normals;
extern cvar_t *r_draw_wireframe;

void R_SortElements(r_element_t *elements, size_t len);
void R_UpdateFrustum(void);
void R_InitView(void);

// render mode function pointers
extern void (*R_DrawOpaqueBspSurfaces)(const r_bsp_surfaces_t *surfs);
extern void (*R_DrawOpaqueWarpBspSurfaces)(const r_bsp_surfaces_t *surfs);
extern void (*R_DrawAlphaTestBspSurfaces)(const r_bsp_surfaces_t *surfs);
extern void (*R_DrawBlendBspSurfaces)(const r_bsp_surfaces_t *surfs);
extern void (*R_DrawBlendWarpBspSurfaces)(const r_bsp_surfaces_t *surfs);
extern void (*R_DrawBackBspSurfaces)(const r_bsp_surfaces_t *surfs);
extern void (*R_DrawMeshModel)(const r_entity_t *e);

#endif /* __R_LOCAL_H__ */

#endif /* __R_MAIN_H__ */

