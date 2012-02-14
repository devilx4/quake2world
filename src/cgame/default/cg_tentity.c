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

/*
 * Cg_BlasterEffect
 */
static void Cg_BlasterEffect(const vec3_t org, const vec3_t dir, int color) {
	r_particle_t *p;
	r_sustained_light_t s;
	int i, j;

	for (i = 0; i < 16; i++) {

		if (!(p = Cg_AllocParticle()))
			break;

		VectorCopy(org, p->org);

		VectorScale(dir, 150.0, p->vel);

		for (j = 0; j < 3; j++) {
			p->vel[j] += crand() * 50.0;
		}

		if (p->vel[2] < 100.0) // deflect up a bit
			p->vel[2] = 100.0;

		p->accel[2] -= 2.0 * PARTICLE_GRAVITY;

		p->image = cg_particle_spark;

		p->color = color + (rand() & 7);

		p->scale = 3.5;
		p->scale_vel = -4.0;

		p->alpha = 1.0;
		p->alpha_vel = -1.0 / (0.7 + crand() * 0.1);
	}

	VectorAdd(org, dir, s.light.origin);
	s.light.radius = 80.0;
	VectorSet(s.light.color, 0.5, 0.3, 0.2);
	s.sustain = 0.25;

	cgi.AddSustainedLight(&s);

	cgi.PlaySample(org, 0, cg_sample_blaster_hit, ATTN_NORM);
}

/*
 * Cg_TracerEffect
 */
static void Cg_TracerEffect(const vec3_t start, const vec3_t end) {
	r_particle_t *p;
	float v;

	if (!(p = Cg_AllocParticle()))
		return;

	p->type = PARTICLE_BEAM;
	p->image = cg_particle_beam;

	p->scale = 1.0;

	VectorCopy(start, p->org);
	VectorCopy(end, p->end);

	VectorSubtract(end, start, p->vel);
	VectorScale(p->vel, 2.0, p->vel);

	v = VectorNormalize(p->vel);
	VectorScale(p->vel, v < 1000.0 ? v : 1000.0, p->vel);

	p->alpha = 0.2;
	p->alpha_vel = -0.4;

	p->color = 14;
}

/*
 * Cg_BulletEffect
 */
static void Cg_BulletEffect(const vec3_t org, const vec3_t dir) {
	static unsigned int last_ric_time;
	r_particle_t *p;
	r_sustained_light_t s;
	vec3_t v;

	if (!(p = Cg_AllocParticle()))
		return;

	p->type = PARTICLE_DECAL;
	VectorAdd(org, dir, p->org);

	VectorScale(dir, -1.0, v);
	VectorAngles(v, p->dir);
	p->dir[ROLL] = rand() % 360;

	p->image = cg_particle_bullet[rand() & 2];

	p->color = 0 + (rand() & 1);

	p->scale = 1.5;

	p->alpha = 2.0;
	p->alpha_vel = -1.0 / (2.0 + frand() * 0.3);

	p->blend = GL_ONE_MINUS_SRC_ALPHA;

	if (!(p = Cg_AllocParticle()))
		return;

	VectorCopy(org, p->org);

	VectorScale(dir, 200.0, p->vel);

	if (p->vel[2] < 100.0) // deflect up a bit
		p->vel[2] = 100.0;

	p->accel[2] -= 4.0 * PARTICLE_GRAVITY;

	p->image = cg_particle_spark;

	p->color = 221 + (rand() & 7);

	p->scale = 1.5;
	p->scale_vel = -4.0;

	p->alpha = 1.0;
	p->alpha_vel = -1.0 / (0.7 + crand() * 0.1);

	VectorAdd(org, dir, s.light.origin);
	s.light.radius = 20.0;
	VectorSet(s.light.color, 0.5, 0.3, 0.2);
	s.sustain = 0.25;

	cgi.AddSustainedLight(&s);

	if (cgi.client->time < last_ric_time)
		last_ric_time = 0;

	if (cgi.client->time - last_ric_time > 300) {
		cgi.PlaySample(org, -1, cg_sample_machinegun_hit[rand() % 3], ATTN_NORM);
		last_ric_time = cgi.client->time;
	}
}

/*
 * Cg_BurnEffect
 */
static void Cg_BurnEffect(const vec3_t org, const vec3_t dir, int scale) {
	r_particle_t *p;
	vec3_t v;

	if (!(p = Cg_AllocParticle()))
		return;

	p->image = cg_particle_burn;
	p->color = 0 + (rand() & 1);
	p->type = PARTICLE_DECAL;
	p->scale = scale;

	VectorScale(dir, -1, v);
	VectorAngles(v, p->dir);
	p->dir[ROLL] = rand() % 360;
	VectorAdd(org, dir, p->org);

	p->alpha = 2.0;
	p->alpha_vel = -1.0 / (2.0 + frand() * 0.3);

	p->blend = GL_ONE_MINUS_SRC_ALPHA;
}

/*
 * Cg_BloodEffect
 */
static void Cg_BloodEffect(const vec3_t org, const vec3_t dir, int count) {
	int i, j;
	r_particle_t *p;
	float d;

	for (i = 0; i < count; i++) {

		if (!(p = Cg_AllocParticle()))
			break;

		p->color = 232 + (rand() & 7);
		p->image = cg_particle_blood;
		p->scale = 6.0;

		d = rand() & 31;
		for (j = 0; j < 3; j++) {
			p->org[j] = org[j] + ((rand() & 7) - 4.0) + d * dir[j];
			p->vel[j] = crand() * 20.0;
		}
		p->org[2] += 16.0 * PM_SCALE;

		p->accel[0] = p->accel[1] = 0.0;
		p->accel[2] = PARTICLE_GRAVITY / 4.0;

		p->alpha = 1.0;
		p->alpha_vel = -1.0 / (0.5 + frand() * 0.3);
	}
}

#define GIB_STREAM_DIST 180.0
#define GIB_STREAM_COUNT 25

/*
 * Cg_GibEffect
 */
void Cg_GibEffect(const vec3_t org, int count) {
	r_particle_t *p;
	vec3_t o, v, tmp;
	c_trace_t tr;
	float dist;
	int i, j;

	// if a player has died underwater, emit some bubbles
	if (cgi.PointContents(org) & MASK_WATER) {
		VectorCopy(org, tmp);
		tmp[2] += 64.0;

		Cg_BubbleTrail(org, tmp, 16.0);
	}

	for (i = 0; i < count; i++) {

		// set the origin and velocity for each gib stream
		VectorSet(o, crand() * 8.0, crand() * 8.0, 8.0 + crand() * 12.0);
		VectorAdd(o, org, o);

		VectorSet(v, crand(), crand(), 0.2 + frand());

		dist = GIB_STREAM_DIST;
		VectorMA(o, dist, v, tmp);

		tr = cgi.Trace(o, tmp, 0.0, MASK_SHOT);
		dist = GIB_STREAM_DIST * tr.fraction;

		for (j = 1; j < GIB_STREAM_COUNT; j++) {

			if (!(p = Cg_AllocParticle()))
				return;

			p->color = 232 + (rand() & 7);
			p->image = cg_particle_blood;

			VectorCopy(o, p->org);

			VectorScale(v, dist * ((float)j / GIB_STREAM_COUNT), p->vel);
			p->vel[0] += crand() * 2.0;
			p->vel[1] += crand() * 2.0;
			p->vel[2] += 100.0;

			p->accel[0] = p->accel[1] = 0.0;
			p->accel[2] = -PARTICLE_GRAVITY * 2.0;

			p->scale = 6.0 + frand();
			p->scale_vel = -6.0 + crand();

			p->alpha = 1.0;
			p->alpha_vel = -1.0 / (2.0 + frand() * 0.3);
		}
	}
}

/*
 * Cg_SparksEffect
 */
void Cg_SparksEffect(const vec3_t org, const vec3_t dir, int count) {
	r_particle_t *p;
	r_sustained_light_t s;
	int i, j;

	for (i = 0; i < count; i++) {

		if (!(p = Cg_AllocParticle()))
			break;

		p->image = cg_particle_spark;

		p->color = 0xe0 + (rand() & 7);

		VectorCopy(org, p->org);
		VectorCopy(dir, p->vel);

		for (j = 0; j < 3; j++) {
			p->org[j] += crand() * 4.0;
			p->vel[j] += crand() * 4.0;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY;

		p->scale = 2.5;
		p->scale_vel = -4.0;

		p->alpha = 1.5;
		p->alpha_vel = -1.0 / (0.5 + frand() * 0.3);
	}

	VectorCopy(org, s.light.origin);
	s.light.radius = 80.0;
	VectorSet(s.light.color, 0.7, 0.5, 0.5);
	s.sustain = 0.65;

	cgi.AddSustainedLight(&s);

	cgi.PlaySample(org, -1, cg_sample_sparks, ATTN_STATIC);
}

/*
 * Cg_ExplosionEffect
 */
static void Cg_ExplosionEffect(const vec3_t org) {
	int i, j;
	r_particle_t *p;
	r_sustained_light_t s;

	if (!(p = Cg_AllocParticle()))
		return;

	p->image = cg_particle_explosion;

	p->scale = 1.0;
	p->scale_vel = 600.0;

	p->alpha = 1.0;
	p->alpha_vel = -4.0;

	p->color = 224;

	VectorCopy(org, p->org);

	if (!(p = Cg_AllocParticle()))
		return;

	i = CONTENTS_SLIME | CONTENTS_WATER;

	if (cgi.PointContents(org) & i)
		return;

	p->accel[2] = 20;

	p->image = cg_particle_smoke;

	p->type = PARTICLE_ROLL;
	p->roll = crand() * 100.0;

	p->scale = 12.0;
	p->scale_vel = 40.0;

	p->alpha = 1.0;
	p->alpha_vel = -1.0 / (1 + frand() * 0.6);

	p->color = rand() & 7;
	p->blend = GL_ONE;

	VectorCopy(org, p->org);
	p->org[2] += 10;

	for (j = 0; j < 3; j++) {
		p->vel[j] = crand();
	}

	VectorCopy(org, s.light.origin);
	s.light.radius = 150.0;
	VectorSet(s.light.color, 0.8, 0.4, 0.2);
	s.sustain = 1.0;

	cgi.AddSustainedLight(&s);

	cgi.PlaySample(org, -1, cg_sample_explosion, ATTN_NORM);
}

/*
 * Cg_HyperblasterEffect
 */
static void Cg_HyperblasterEffect(const vec3_t org) {
	r_sustained_light_t s;

	VectorCopy(org, s.light.origin);
	s.light.radius = 80.0;
	VectorSet(s.light.color, 0.4, 0.7, 1.0);
	s.sustain = 0.25;

	cgi.AddSustainedLight(&s);

	cgi.PlaySample(org, -1, cg_sample_hyperblaster_hit, ATTN_NORM);
}

/*
 * Cg_LightningEffect
 */
static void Cg_LightningEffect(const vec3_t org) {
	r_sustained_light_t s;
	vec3_t tmp;
	int i, j;

	for (i = 0; i < 40; i++) {

		VectorCopy(org, tmp);

		for (j = 0; j < 3; j++)
			tmp[j] = tmp[j] + (rand() % 96) - 48;

		Cg_BubbleTrail(org, tmp, 4.0);
	}

	VectorCopy(org, s.light.origin);
	s.light.radius = 160.0;
	VectorSet(s.light.color, 0.6, 0.6, 1.0);
	s.sustain = 0.75;

	cgi.AddSustainedLight(&s);

	cgi.PlaySample(org, -1, cg_sample_lightning_discharge, ATTN_NORM);
}

/*
 * Cg_RailEffect
 */
static void Cg_RailEffect(const vec3_t start, const vec3_t end, int flags, int color) {
	vec3_t vec, move;
	float len;
	r_particle_t *p;
	r_sustained_light_t s;
	int i;

	VectorCopy(start, s.light.origin);
	s.light.radius = 100.0;
	cgi.ColorFromPalette(color, s.light.color);
	s.sustain = 0.75;

	cgi.AddSustainedLight(&s);

	if (!(p = Cg_AllocParticle()))
		return;

	// draw the core with a beam
	p->type = PARTICLE_BEAM;
	p->image = cg_particle_beam;
	p->scale = 3.0;

	VectorCopy(start, p->org);
	VectorCopy(end, p->end);

	p->alpha = 0.75;
	p->alpha_vel = -0.75;

	// white cores for some colors, shifted for others
	p->color = (color > 239 || color == 187 ? 216 : color + 6);

	// and the spiral with normal parts
	VectorCopy(start, move);

	VectorSubtract(end, start, vec);
	len = VectorNormalize(vec);

	for (i = 0; i < len; i += 24) {

		if (!(p = Cg_AllocParticle()))
			return;

		p->type = PARTICLE_ROLL;
		p->roll = crand() * 300.0;

		VectorMA(start, i, vec, p->org);

		VectorSet(p->vel, 0.0, 0.0, 2.0);

		p->image = cg_particle_rail_trail;

		p->alpha = 1.0;
		p->alpha_vel = -1.0;

		p->scale = 3.0 + crand() * 0.3;
		p->scale_vel = 8.0 + crand() * 0.3;

		p->color = color;

		// check for bubble trail
		if (i && (cgi.PointContents(move) & (CONTENTS_SLIME | CONTENTS_WATER))) {
			Cg_BubbleTrail(move, p->org, 16.0);
		}

		VectorCopy(p->org, move);
	}

	// check for explosion effect on solids
	if (flags & SURF_SKY)
		return;

	if (!(p = Cg_AllocParticle()))
		return;

	p->image = cg_particle_explosion;

	p->scale = 1.0;
	p->scale_vel = 400.0;

	p->alpha = 2.0;
	p->alpha_vel = -10.0;

	p->color = color;

	VectorCopy(end, p->org);

	VectorMA(end, -12.0, vec, s.light.origin);
	s.light.radius = 120.0;
	s.sustain = 1.25;

	cgi.AddSustainedLight(&s);
}

/*
 * Cg_BfgEffect
 */
static void Cg_BfgEffect(const vec3_t org) {
	r_particle_t *p;
	r_sustained_light_t s;
	int i;

	for (i = 0; i < 4; i++) {

		if (!(p = Cg_AllocParticle()))
			return;

		p->image = cg_particle_explosion;

		p->scale = 1.0;
		p->scale_vel = 200.0 * (i + 1);

		p->alpha = 1.0;
		p->alpha_vel = -3.0;

		p->color = 201;

		VectorCopy(org, p->org);
	}

	VectorCopy(org, s.light.origin);
	s.light.radius = 200.0;
	VectorSet(s.light.color, 0.8, 1.0, 0.5);
	s.sustain = 1.0;

	cgi.AddSustainedLight(&s);

	cgi.PlaySample(org, -1, cg_sample_bfg_hit, ATTN_NORM);
}

/*
 * Cg_ParseTempEntity
 */
void Cg_ParseTempEntity(void) {
	vec3_t pos, pos2, dir;
	int i, j;

	const byte type = cgi.ReadByte();

	switch (type) {

	case TE_BLASTER:
		cgi.ReadPosition(pos);
		cgi.ReadPosition(dir);
		i = cgi.ReadByte();
		Cg_BlasterEffect(pos, dir, i);
		break;

	case TE_TRACER:
		cgi.ReadPosition(pos);
		cgi.ReadPosition(pos2);
		Cg_TracerEffect(pos, pos2);
		break;

	case TE_BULLET: // bullet hitting wall
		cgi.ReadPosition(pos);
		cgi.ReadDirection(dir);
		Cg_BulletEffect(pos, dir);
		break;

	case TE_BURN: // burn mark on wall
		cgi.ReadPosition(pos);
		cgi.ReadDirection(dir);
		i = cgi.ReadByte();
		Cg_BurnEffect(pos, dir, i);
		break;

	case TE_BLOOD: // projectile hitting flesh
		cgi.ReadPosition(pos);
		cgi.ReadDirection(dir);
		Cg_BloodEffect(pos, dir, 12);
		break;

	case TE_GIB: // player over-death
		cgi.ReadPosition(pos);
		Cg_GibEffect(pos, 12);
		break;

	case TE_SPARKS: // colored sparks
		cgi.ReadPosition(pos);
		cgi.ReadDirection(dir);
		Cg_SparksEffect(pos, dir, 12);
		break;

	case TE_HYPERBLASTER: // hyperblaster hitting wall
		cgi.ReadPosition(pos);
		Cg_HyperblasterEffect(pos);
		break;

	case TE_LIGHTNING: // lightning discharge in water
		cgi.ReadPosition(pos);
		Cg_LightningEffect(pos);
		break;

	case TE_RAIL: // railgun effect
		cgi.ReadPosition(pos);
		cgi.ReadPosition(pos2);
		i = cgi.ReadLong();
		j = cgi.ReadByte();
		Cg_RailEffect(pos, pos2, i, j);
		break;

	case TE_EXPLOSION: // rocket and grenade explosions
		cgi.ReadPosition(pos);
		Cg_ExplosionEffect(pos);
		break;

	case TE_BFG: // bfg explosion
		cgi.ReadPosition(pos);
		Cg_BfgEffect(pos);
		break;

	case TE_BUBBLES: // bubbles chasing projectiles in water
		cgi.ReadPosition(pos);
		cgi.ReadPosition(pos2);
		Cg_BubbleTrail(pos, pos2, 1.0);
		break;

	default:
		cgi.Warn("Cg_ParseTempEntity: Unknown type: %d\n.", type);
		return;
	}
}