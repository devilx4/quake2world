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

static g_client_score_t cg_scores[MAX_CLIENTS];
static size_t cg_num_scores;

/*
 * Cg_ParseScores
 */
void Cg_ParseScores(void) {

	const size_t len = cgi.ReadShort();
	cgi.ReadData((void *) cg_scores, len);

	cg_num_scores = len / sizeof(g_client_score_t);
}

/*
 * Cg_DrawScores
 */
void Cg_DrawScores(player_state_t *ps) {
	char string[MAX_QPATH];
	r_pixel_t cw, ch;
	size_t i;

	if (!ps->stats[STAT_SCORES])
		return;

	cgi.BindFont("small", &cw, &ch);

	for (i = 0; i < cg_num_scores; i++) {
		const g_client_score_t *s = &cg_scores[i];

		const r_pixel_t x = 100;
		const r_pixel_t y = 100 + i * ch;

		snprintf(string, sizeof(string) - 1, "%d: %d frags %3dms",
				s->entity_num, s->score, s->ping);
		cgi.DrawString(x, y, string, CON_COLOR_WHITE);
	}
}