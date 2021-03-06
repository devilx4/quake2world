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

#include "bspfile.h"
#include "polylib.h"

uint32_t c_peak_windings;

#define	BOGUS_RANGE	8192

/*
 * @brief
 */
winding_t *AllocWinding(int32_t points) {

	if (debug) {
		SDL_SemPost(semaphores.active_windings);
		uint32_t active_windings = SDL_SemValue(semaphores.active_windings);

		if (active_windings > c_peak_windings) {
			c_peak_windings = active_windings;
		}
	}

	return Z_Malloc(sizeof(int32_t) + sizeof(vec3_t) * points);
}

/*
 * @brief
 */
void FreeWinding(winding_t *w) {

	if (debug)
		SDL_SemWait(semaphores.active_windings);

	Z_Free(w);
}

/*
 * @brief
 */
void RemoveColinearPoints(winding_t *w) {
	int32_t i;
	vec3_t v1, v2;
	int32_t nump;
	vec3_t p[MAX_POINTS_ON_WINDING];

	nump = 0;
	for (i = 0; i < w->numpoints; i++) {
		const int32_t j = (i + 1) % w->numpoints;
		const int32_t k = (i + w->numpoints - 1) % w->numpoints;
		VectorSubtract(w->p[j], w->p[i], v1);
		VectorSubtract(w->p[i], w->p[k], v2);
		VectorNormalize(v1);
		VectorNormalize(v2);
		if (DotProduct(v1, v2) < 0.999) {
			VectorCopy(w->p[i], p[nump]);
			nump++;
		}
	}

	if (nump == w->numpoints)
		return;

	if (debug) {
		const int32_t j = w->numpoints - nump;
		for (i = 0; i < j; i++) {
			SDL_SemPost(semaphores.removed_points);
		}
	}

	w->numpoints = nump;
	memcpy(w->p, p, nump * sizeof(p[0]));
}

/*
 * @brief
 */
void WindingPlane(const winding_t *w, vec3_t normal, vec_t *dist) {
	vec3_t v1, v2;

	VectorSubtract(w->p[1], w->p[0], v1);
	VectorSubtract(w->p[2], w->p[0], v2);
	CrossProduct(v2, v1, normal);
	VectorNormalize(normal);
	*dist = DotProduct(w->p[0], normal);
}

/*
 * @brief
 */
vec_t WindingArea(const winding_t *w) {
	int32_t i;
	vec3_t d1, d2, cross;
	vec_t total;

	total = 0;
	for (i = 2; i < w->numpoints; i++) {
		VectorSubtract(w->p[i - 1], w->p[0], d1);
		VectorSubtract(w->p[i], w->p[0], d2);
		CrossProduct(d1, d2, cross);
		total += 0.5 * VectorLength(cross);
	}
	return total;
}

/*
 * @brief
 */
void WindingBounds(const winding_t *w, vec3_t mins, vec3_t maxs) {
	int32_t i, j;

	mins[0] = mins[1] = mins[2] = 99999;
	maxs[0] = maxs[1] = maxs[2] = -99999;

	for (i = 0; i < w->numpoints; i++) {
		for (j = 0; j < 3; j++) {
			const vec_t v = w->p[i][j];
			if (v < mins[j])
				mins[j] = v;
			if (v > maxs[j])
				maxs[j] = v;
		}
	}
}

/*
 * @brief
 */
void WindingCenter(const winding_t *w, vec3_t center) {
	int32_t i;
	vec_t scale;

	VectorCopy(vec3_origin, center);
	for (i = 0; i < w->numpoints; i++)
		VectorAdd(w->p[i], center, center);

	scale = 1.0 / w->numpoints;
	VectorScale(center, scale, center);
}

/*
 * @brief
 */
winding_t *WindingForPlane(const vec3_t normal, const vec_t dist) {
	int32_t i, x;
	vec_t max, v;
	vec3_t org, vright, vup;
	winding_t *w;

	// find the major axis
	max = -BOGUS_RANGE;
	x = -1;
	for (i = 0; i < 3; i++) {
		v = fabs(normal[i]);
		if (v > max) {
			x = i;
			max = v;
		}
	}
	if (x == -1)
		Com_Error(ERR_FATAL, "No axis found\n");

	VectorCopy(vec3_origin, vup);
	switch (x) {
		case 0:
		case 1:
			vup[2] = 1;
			break;
		case 2:
			vup[0] = 1;
			break;
	}

	v = DotProduct(vup, normal);
	VectorMA(vup, -v, normal, vup);
	VectorNormalize(vup);

	VectorScale(normal, dist, org);

	CrossProduct(vup, normal, vright);

	VectorScale(vup, 8192, vup);
	VectorScale(vright, 8192, vright);

	// project a really big	axis aligned box onto the plane
	w = AllocWinding(4);

	VectorSubtract(org, vright, w->p[0]);
	VectorAdd(w->p[0], vup, w->p[0]);

	VectorAdd(org, vright, w->p[1]);
	VectorAdd(w->p[1], vup, w->p[1]);

	VectorAdd(org, vright, w->p[2]);
	VectorSubtract(w->p[2], vup, w->p[2]);

	VectorSubtract(org, vright, w->p[3]);
	VectorSubtract(w->p[3], vup, w->p[3]);

	w->numpoints = 4;

	return w;
}

/*
 * @brief
 */
winding_t *WindingForFace(const d_bsp_face_t * f) {
	int32_t i;
	d_bsp_vertex_t *dv;
	int32_t v;
	winding_t *w;

	w = AllocWinding(f->num_edges);
	w->numpoints = f->num_edges;

	for (i = 0; i < f->num_edges; i++) {
		const int32_t se = d_bsp.face_edges[f->first_edge + i];
		if (se < 0)
			v = d_bsp.edges[-se].v[1];
		else
			v = d_bsp.edges[se].v[0];

		dv = &d_bsp.vertexes[v];
		VectorCopy(dv->point, w->p[i]);
	}

	RemoveColinearPoints(w);

	return w;
}

/*
 * @brief
 */
winding_t *CopyWinding(const winding_t *w) {
	size_t size;
	winding_t *c;

	c = AllocWinding(w->numpoints);
	size = (size_t) ((winding_t *) 0)->p[w->numpoints];
	memcpy(c, w, size);
	return c;
}

/*
 * @brief
 */
winding_t *ReverseWinding(winding_t *w) {
	int32_t i;
	winding_t *c;

	c = AllocWinding(w->numpoints);
	for (i = 0; i < w->numpoints; i++) {
		VectorCopy(w->p[w->numpoints-1-i], c->p[i]);
	}
	c->numpoints = w->numpoints;
	return c;
}

/*
 * @brief
 */
void ClipWindingEpsilon(const winding_t *in, vec3_t normal, vec_t dist, vec_t epsilon,
		winding_t **front, winding_t **back) {
	vec_t dists[MAX_POINTS_ON_WINDING + 4];
	int32_t sides[MAX_POINTS_ON_WINDING + 4];
	int32_t counts[SIDE_BOTH + 1];
	static vec_t dot; // VC 4.2 optimizer bug if not static
	int32_t i, j;
	const vec_t *p2;
	vec3_t mid;
	winding_t *f, *b;
	int32_t maxpts;

	memset(counts, 0, sizeof(counts));

	// determine sides for each point
	for (i = 0; i < in->numpoints; i++) {
		dot = DotProduct(in->p[i], normal);
		dot -= dist;
		dists[i] = dot;
		if (dot > epsilon)
			sides[i] = SIDE_FRONT;
		else if (dot < -epsilon)
			sides[i] = SIDE_BACK;
		else {
			sides[i] = SIDE_BOTH;
		}
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];

	*front = *back = NULL;

	if (!counts[SIDE_FRONT]) {
		*back = CopyWinding(in);
		return;
	}
	if (!counts[SIDE_BACK]) {
		*front = CopyWinding(in);
		return;
	}

	maxpts = in->numpoints + 4;

	*front = f = AllocWinding(maxpts);
	*back = b = AllocWinding(maxpts);

	for (i = 0; i < in->numpoints; i++) {
		const vec_t *p1 = in->p[i];

		if (sides[i] == SIDE_BOTH) {
			VectorCopy(p1, f->p[f->numpoints]);
			f->numpoints++;
			VectorCopy(p1, b->p[b->numpoints]);
			b->numpoints++;
			continue;
		}

		if (sides[i] == SIDE_FRONT) {
			VectorCopy(p1, f->p[f->numpoints]);
			f->numpoints++;
		}
		if (sides[i] == SIDE_BACK) {
			VectorCopy(p1, b->p[b->numpoints]);
			b->numpoints++;
		}

		if (sides[i + 1] == SIDE_BOTH || sides[i + 1] == sides[i])
			continue;

		// generate a split point
		p2 = in->p[(i + 1) % in->numpoints];

		dot = dists[i] / (dists[i] - dists[i + 1]);
		for (j = 0; j < 3; j++) { // avoid round off error when possible
			if (normal[j] == 1)
				mid[j] = dist;
			else if (normal[j] == -1)
				mid[j] = -dist;
			else
				mid[j] = p1[j] + dot * (p2[j] - p1[j]);
		}

		VectorCopy(mid, f->p[f->numpoints]);
		f->numpoints++;
		VectorCopy(mid, b->p[b->numpoints]);
		b->numpoints++;
	}

	if (f->numpoints > maxpts || b->numpoints > maxpts)
		Com_Error(ERR_FATAL, "Points exceeded estimate\n");
	if (f->numpoints > MAX_POINTS_ON_WINDING || b->numpoints > MAX_POINTS_ON_WINDING)
		Com_Error(ERR_FATAL, "MAX_POINTS_ON_WINDING\n");
}

/*
 * @brief
 */
void ChopWindingInPlace(winding_t **inout, const vec3_t normal, const vec_t dist,
		const vec_t epsilon) {
	winding_t *in;
	vec_t dists[MAX_POINTS_ON_WINDING + 4];
	int32_t sides[MAX_POINTS_ON_WINDING + 4];
	int32_t counts[SIDE_BOTH + 1];
	static vec_t dot; // VC 4.2 optimizer bug if not static
	int32_t i, j;
	vec_t *p1, *p2;
	vec3_t mid;
	winding_t *f;
	int32_t maxpts;

	in = *inout;

	memset(counts, 0, sizeof(counts));

	// determine sides for each point
	for (i = 0; i < in->numpoints; i++) {
		dot = DotProduct(in->p[i], normal);
		dot -= dist;
		dists[i] = dot;
		if (dot > epsilon)
			sides[i] = SIDE_FRONT;
		else if (dot < -epsilon)
			sides[i] = SIDE_BACK;
		else {
			sides[i] = SIDE_BOTH;
		}
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];

	if (!counts[SIDE_FRONT]) {
		FreeWinding(in);
		*inout = NULL;
		return;
	}
	if (!counts[SIDE_BACK])
		return; // inout stays the same

	maxpts = in->numpoints + 4;

	f = AllocWinding(maxpts);

	for (i = 0; i < in->numpoints; i++) {
		p1 = in->p[i];

		if (sides[i] == SIDE_BOTH) {
			VectorCopy(p1, f->p[f->numpoints]);
			f->numpoints++;
			continue;
		}

		if (sides[i] == SIDE_FRONT) {
			VectorCopy(p1, f->p[f->numpoints]);
			f->numpoints++;
		}

		if (sides[i + 1] == SIDE_BOTH || sides[i + 1] == sides[i])
			continue;

		// generate a split point
		p2 = in->p[(i + 1) % in->numpoints];

		dot = dists[i] / (dists[i] - dists[i + 1]);
		for (j = 0; j < 3; j++) { // avoid round off error when possible
			if (normal[j] == 1)
				mid[j] = dist;
			else if (normal[j] == -1)
				mid[j] = -dist;
			else
				mid[j] = p1[j] + dot * (p2[j] - p1[j]);
		}

		VectorCopy(mid, f->p[f->numpoints]);
		f->numpoints++;
	}

	if (f->numpoints > maxpts)
		Com_Error(ERR_FATAL, "Points exceeded estimate\n");
	if (f->numpoints > MAX_POINTS_ON_WINDING)
		Com_Error(ERR_FATAL, "MAX_POINTS_ON_WINDING\n");

	FreeWinding(in);
	*inout = f;
}

/*
 * @brief Returns the fragment of in that is on the front side
 * of the cliping plane. The original is freed.
 */
winding_t *ChopWinding(winding_t *in, vec3_t normal, vec_t dist) {
	winding_t *f, *b;

	ClipWindingEpsilon(in, normal, dist, ON_EPSILON, &f, &b);
	FreeWinding(in);
	if (b)
		FreeWinding(b);
	return f;
}

