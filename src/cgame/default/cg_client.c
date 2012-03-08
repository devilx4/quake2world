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

#include "cg_local.h"

#define DEFAULT_CLIENT_INFO "newbie\\qforcer/enforcer"

/*
 * Cg_ValidateClient
 */
static bool Cg_ValidateClient(cl_client_info_t *ci) {

	if (!ci->head || !ci->upper || !ci->lower)
		return false;

	if (ci->head_skin->type == it_null || ci->upper_skin->type == it_null || ci->lower_skin->type
			== it_null) {
		return false;
	}

	return true;
}

/*
 * Cg_LoadClient
 */
void Cg_LoadClient(cl_client_info_t *ci, const char *s) {
	char model_name[MAX_QPATH];
	char skin_name[MAX_QPATH];
	char path[MAX_QPATH];
	const char *t;
	char *u, *v;
	int i;

	// copy the entire string
	strncpy(ci->info, s, sizeof(ci->info));
	ci->info[sizeof(ci->info) - 1] = '\0';

	i = 0;
	t = s;
	while (*t) { // check for non-printable chars
		if (*t <= 32) {
			i = -1;
			break;
		}
		t++;
	}

	if (*ci->info == '\0' || i == -1) { // use default
		Cg_LoadClient(ci, DEFAULT_CLIENT_INFO);
		return;
	}

	// isolate the player's name
	strncpy(ci->name, s, sizeof(ci->name));
	ci->name[sizeof(ci->name) - 1] = '\0';

	v = strchr(ci->name, '\\');
	u = strchr(ci->name, '/');

	if (v && u && (v < u)) { // valid
		*v = *u = 0;
		strcpy(model_name, v + 1);
		strcpy(skin_name, u + 1);
	} else { // invalid
		Cg_LoadClient(ci, DEFAULT_CLIENT_INFO);
		return;
	}

	// load the models
	snprintf(path, sizeof(path), "players/%s/head.md3", model_name);
	ci->head = cgi.LoadModel(path);

	snprintf(path, sizeof(path), "players/%s/upper.md3", model_name);
	ci->upper = cgi.LoadModel(path);

	snprintf(path, sizeof(path), "players/%s/lower.md3", model_name);
	ci->lower = cgi.LoadModel(path);

	// and the skins
	snprintf(path, sizeof(path), "players/%s/%s_h.tga", model_name, skin_name);
	ci->head_skin = cgi.LoadImage(path, it_skin);

	snprintf(path, sizeof(path), "players/%s/%s_u.tga", model_name, skin_name);
	ci->upper_skin = cgi.LoadImage(path, it_skin);

	snprintf(path, sizeof(path), "players/%s/%s_l.tga", model_name, skin_name);
	ci->lower_skin = cgi.LoadImage(path, it_skin);

	// ensure we were able to load everything
	if (!Cg_ValidateClient(ci)) {
		Cg_LoadClient(ci, DEFAULT_CLIENT_INFO);
		return;
	}

	cgi.Debug("Loaded cl_client_info_t: %s\n", ci->info);
}

/*
 * Cg_LoadClients
 *
 * Load all client info strings from the server.
 */
void Cg_LoadClients(void) {
	int i;

	for (i = 0; i < MAX_CLIENTS; i++) {
		cl_client_info_t *ci = &cgi.client->client_info[i];
		const char *s = cgi.ConfigString(CS_CLIENTS + i);

		if (!*s)
			continue;

		Cg_LoadClient(ci, s);
	}
}

/*
 * Cg_NextAnimation
 *
 * Returns the next animation to advance to, defaulting to a no-op.
 */
static entity_animation_t Cg_NextAnimation(const entity_animation_t a) {

	switch (a) {
	case ANIM_BOTH_DEATH1:
	case ANIM_BOTH_DEATH2:
	case ANIM_BOTH_DEATH3:
		return a + 1;

	case ANIM_TORSO_GESTURE:
	case ANIM_TORSO_ATTACK1:
	case ANIM_TORSO_ATTACK2:
	case ANIM_TORSO_DROP:
	case ANIM_TORSO_RAISE:
		return ANIM_TORSO_STAND1;

	case ANIM_LEGS_LAND1:
	case ANIM_LEGS_LAND2:
		return ANIM_LEGS_IDLE;

	default:
		return a;
	}
}

/*
 * Cg_AnimateClientEntity_
 *
 * Resolve the frames and interpolation fractions for the specified animation
 * and entity. If a non-looping animation has completed, proceed to the next
 * animation in the sequence.
 */
static void Cg_AnimateClientEntity_(const r_md3_t *md3, cl_entity_animation_t *a, r_entity_t *e) {

	e->frame = e->old_frame = 0;
	e->lerp = 1.0;
	e->back_lerp = 0.0;

	if (a->animation > md3->num_animations) {
		cgi.Warn("Cg_AnimateClientEntity: Invalid animation: %s: %d\n", e->model->name,
				a->animation);
		return;
	}

	const r_md3_animation_t *anim = &md3->animations[a->animation];

	if (!anim->num_frames || !anim->hz) {
		cgi.Warn("Cg_AnimateClientEntity_: Bad animation sequence: %s: %d\n", e->model->name,
				a->animation);
		return;
	}

	const int frame_time = 1000 / anim->hz;
	const int animation_time = anim->num_frames * frame_time;
	const int elapsed_time = cgi.client->time - a->time;
	int frame = elapsed_time / frame_time;

	if (elapsed_time >= animation_time) { // to loop, or not to loop

		if (!anim->looped_frames) {
			const entity_animation_t next = Cg_NextAnimation(a->animation);

			if (next == a->animation) { // no change, just stay put
				e->frame = anim->first_frame + anim->num_frames - 1;
				e->lerp = 1.0;
				e->back_lerp = 0.0;
				return;
			}

			a->animation = next; // or move into the next animation
			a->time = cgi.client->time;

			Cg_AnimateClientEntity_(md3, a, e);
			return;
		}

		frame = (frame - anim->num_frames) % anim->looped_frames;
	}

	frame = anim->first_frame + frame;

	if (frame != a->frame) { // shuffle the frames
		a->old_frame = a->frame;
		a->frame = frame;
	}

	a->lerp = (elapsed_time % frame_time) / (float) frame_time;
	a->fraction = elapsed_time / (float) animation_time;

	e->frame = a->frame;
	e->old_frame = a->old_frame;
	e->lerp = a->lerp;
	e->back_lerp = 1.0 - a->lerp;
}

/*
 * Cg_AnimateClientEntity
 *
 * Runs the animation sequences for the specified entity, setting the frame
 * indexes and interpolation fractions for the specified renderer entities.
 */
void Cg_AnimateClientEntity(cl_entity_t *e, r_entity_t *upper, r_entity_t *lower) {
	const r_md3_t *md3 = (r_md3_t *) upper->model->extra_data;

	// do the torso animation
	if (e->current.animation1 != e->prev.animation1 || !e->animation1.time) {
		//cgi.Debug("torso: %d -> %d\n", e->current.animation1, e->prev.animation1);
		e->animation1.animation = e->current.animation1 & ~ANIM_TOGGLE_BIT;
		e->animation1.time = cgi.client->time;
	}

	Cg_AnimateClientEntity_(md3, &e->animation1, upper);

	// and then the legs
	if (e->current.animation2 != e->prev.animation2 || !e->animation2.time) {
		//cgi.Debug("legs: %d -> %d\n", e->current.animation2, e->prev.animation2);
		e->animation2.animation = e->current.animation2 & ~ANIM_TOGGLE_BIT;
		e->animation2.time = cgi.client->time;
	}

	Cg_AnimateClientEntity_(md3, &e->animation2, lower);
}
