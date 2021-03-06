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

/*
 * @brief Clears all volatile view members so that a new scene may be populated.
 */
static void Cl_ClearView(void) {

	// reset entity, light, particle and corona counts
	r_view.num_entities = r_view.num_lights = 0;
	r_view.num_particles = r_view.num_coronas = 0;

	// reset counters
	r_view.num_bind_texture = r_view.num_bind_lightmap = r_view.num_bind_deluxemap = 0;
	r_view.num_bind_normalmap = r_view.num_bind_glossmap = 0;

	r_view.num_bsp_surfaces = 0;

	r_view.num_mesh_models = r_view.num_mesh_tris = 0;
}

/*
 * @brief
 */
static void Cl_UpdateViewSize(void) {
	int32_t size;

	if (!cl_view_size->modified && !r_view.update)
		return;

	if (cl_view_size->value < 40.0)
		Cvar_Set("cl_view_size", "40.0");
	if (cl_view_size->value > 100.0)
		Cvar_Set("cl_view_size", "100.0");

	size = cl_view_size->value;

	r_view.width = r_context.width * size / 100.0;
	r_view.height = r_context.height * size / 100.0;

	r_view.x = (r_context.width - r_view.width) / 2.0;
	r_view.y = (r_context.height - r_view.height) / 2.0;

	cl_view_size->modified = false;
}

/*
 * @brief Updates the interpolation fraction for the current client frame.
 * Because the client typically runs at a higher framerate than the server, we
 * use linear interpolation between the last 2 server frames. We aim to reach
 * the current server time just as a new packet arrives.
 */
static void Cl_UpdateLerp(cl_frame_t *from) {

	if (time_demo->value) {
		cl.time = cl.frame.server_time;
		cl.lerp = 1.0;
		return;
	}

	if (cl.time > cl.frame.server_time) {
		Com_Debug("High clamp: %dms\n", cl.time - cl.frame.server_time);
		cl.time = cl.frame.server_time;
		cl.lerp = 1.0;
	} else if (cl.time < from->server_time) {
		Com_Debug("Low clamp: %dms\n", from->server_time - cl.time);
		cl.time = from->server_time;
		cl.lerp = 0.0;
	} else {
		const vec_t delta = cl.time - from->server_time;
		const vec_t interval = cl.frame.server_time - from->server_time;

		if (interval <= 0.0) {
			Com_Debug("Bad clamp\n");
			cl.lerp = 1.0;
			return;
		}

		cl.lerp = delta / interval;
	}
}

/*
 * @brief The origin is typically calculated using client sided prediction, provided
 * the client is not viewing a demo, playing in 3rd person mode, or chasing
 * another player.
 */
static void Cl_UpdateOrigin(player_state_t *ps, player_state_t *ops) {

	if (Cl_UsePrediction()) {
		int32_t i;

		// use client sided prediction
		for (i = 0; i < 3; i++) {
			r_view.origin[i] = cl.predicted_origin[i] + cl.predicted_offset[i];
			r_view.origin[i] -= (1.0 - cl.lerp) * cl.prediction_error[i];
		}

		const vec_t delta = cls.real_time - cl.predicted_step_time;
		const vec_t lerp = cl.predicted_step_lerp;
		if (delta < lerp) { // interpolate stair traversal
			r_view.origin[2] -= cl.predicted_step * ((lerp - delta) / lerp);
		}
	} else { // just use interpolated values from frame
		vec3_t old_origin, current_origin, origin;
		vec3_t old_offset, current_offset, offset;

		UnpackPosition(ops->pm_state.origin, old_origin);
		UnpackPosition(ps->pm_state.origin, current_origin);

		VectorLerp(old_origin, current_origin, cl.lerp, origin);

		UnpackPosition(ops->pm_state.view_offset, old_offset);
		UnpackPosition(ps->pm_state.view_offset, current_offset);

		VectorLerp(old_offset, current_offset, cl.lerp, offset);

		VectorAdd(origin, offset, r_view.origin);
	}

	// update the contents mask for e.g. under-water effects
	r_view.contents = R_PointContents(r_view.origin);
}

/*
 * @brief The angles are typically fetched from input, after factoring in client-side
 * prediction, unless the client is watching a demo or chase camera.
 */
static void Cl_UpdateAngles(player_state_t *ps, player_state_t *ops) {
	vec3_t old_angles, new_angles, angles;

	// start with the predicted angles, or interpolate the server states
	if (Cl_UsePrediction()) {
		VectorCopy(cl.predicted_angles, r_view.angles);
	} else {
		UnpackAngles(ops->pm_state.view_angles, old_angles);
		UnpackAngles(ps->pm_state.view_angles, new_angles);

		AngleLerp(old_angles, new_angles, cl.lerp, r_view.angles);
	}

	// add in the kick angles
	UnpackAngles(ops->pm_state.kick_angles, old_angles);
	UnpackAngles(ps->pm_state.kick_angles, new_angles);

	AngleLerp(old_angles, new_angles, cl.lerp, angles);
	VectorAdd(r_view.angles, angles, r_view.angles);

	// and lastly the delta angles
	UnpackAngles(ops->pm_state.delta_angles, old_angles);
	UnpackAngles(ps->pm_state.delta_angles, new_angles);

	VectorCopy(new_angles, angles);

	// check for small delta angles, and interpolate them
	if (!VectorCompare(new_angles, new_angles)) {

		VectorSubtract(old_angles, new_angles, angles);
		vec_t f = VectorLength(angles);

		if (f < 15.0) {
			AngleLerp(old_angles, new_angles, cl.lerp, angles);
		}
	}

	VectorAdd(r_view.angles, angles, r_view.angles);

	ClampAngles(r_view.angles);

	if (cl.frame.ps.pm_state.pm_type == PM_DEAD) { // look only on x axis
		r_view.angles[0] = 0.0;
		r_view.angles[2] = 45.0;
	}

	// and finally set the view directional vectors
	AngleVectors(r_view.angles, r_view.forward, r_view.right, r_view.up);
}

/*
 * @brief Updates the r_view_t for the renderer. Origin, angles, etc are calculated.
 * Scene population is then delegated to the client game.
 */
void Cl_UpdateView(void) {
	cl_frame_t *prev;
	player_state_t *ps, *ops;

	if (!cl.frame.valid && !r_view.update)
		return; // not a valid frame, and no forced update

	// find the previous frame to interpolate from
	prev = &cl.frames[(cl.frame.server_frame - 1) & UPDATE_MASK];

	if (prev->server_frame != cl.frame.server_frame - 1 || !prev->valid)
		prev = &cl.frame; // previous frame was dropped or invalid

	Cl_UpdateLerp(prev);

	ps = &cl.frame.ps;
	ops = &prev->ps;

	if (ps != ops) { // see if we've teleported
		vec3_t org, old_org, delta;

		UnpackPosition(ps->pm_state.origin, org);
		UnpackPosition(ops->pm_state.origin, old_org);

		VectorSubtract(org, old_org, delta);

		if (VectorLength(delta) > 256.0) {
			ops = ps; // don't interpolate
		}
	}

	Cl_ClearView();

	Cl_UpdateOrigin(ps, ops);

	Cl_UpdateAngles(ps, ops);

	Cl_UpdateViewSize();

	cls.cgame->UpdateView(&cl.frame);

	// set time
	r_view.time = cl.time;

	// set area bits to mark visible leafs
	r_view.area_bits = cl.frame.area_bits;

	// create the thread which populates the view
	r_view.thread = Thread_Create((ThreadRunFunc) cls.cgame->PopulateView, &cl.frame);
}

/*
 * @brief
 */
static void Cl_ViewSizeUp_f(void) {
	Cvar_SetValue("cl_view_size", cl_view_size->integer + 10);
}

/*
 * @brief
 */
static void Cl_ViewSizeDown_f(void) {
	Cvar_SetValue("cl_view_size", cl_view_size->integer - 10);
}

/*
 * @brief
 */
void Cl_InitView(void) {
	Cmd_Add("view_size_up", Cl_ViewSizeUp_f, CMD_CLIENT, NULL);
	Cmd_Add("view_size_down", Cl_ViewSizeDown_f, CMD_CLIENT, NULL);
}
