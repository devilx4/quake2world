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

#define SCORES_COL_WIDTH 240
#define SCORES_ROW_HEIGHT 48
#define SCORES_ICON_WIDTH 48

static player_score_t cg_scores[MAX_CLIENTS];
static size_t cg_num_scores;

static _Bool cg_scores_teams;
static _Bool cg_scores_ctf;

/*
 * @brief A comparator for sorting player_score_t.
 */
static int32_t Cg_ParseScores_Compare(const void *a, const void *b) {
	const player_score_t *sa = (player_score_t *) a;
	const player_score_t *sb = (player_score_t *) b;

	// push spectators to the bottom of the board
	const int16_t s1 = (sa->team == 0xff ? -9999 : sa->score);
	const int16_t s2 = (sb->team == 0xff ? -9999 : sb->score);

	return s2 - s1;
}

/*
 * @brief
 */
void Cg_ParseScores(void) {

	const size_t len = cgi.ReadShort();
	cgi.ReadData((void *) cg_scores, len);

	cg_num_scores = len / sizeof(player_score_t);

	cg_scores_teams = atoi(cgi.ConfigString(CS_TEAMS));
	cg_scores_ctf = atoi(cgi.ConfigString(CS_CTF));

	// the last two scores in the sequence are the team scores
	if (cg_scores_teams || cg_scores_ctf)
		cg_num_scores -= 2;

	/*
	 // to test the scoreboard, uncomment this block
	 size_t i;
	 for (i = cg_num_scores; i < 16; i++) {
	 cg_scores[i].player_num = cg_scores[cg_num_scores - 1].player_num;
	 cg_scores[i].ping = i;
	 cg_scores[i].score = i;
	 cg_scores[i].captures = i;

	 if (i % 6 == 0) { // some spectators
	 cg_scores[i].team = 0xff;
	 cg_scores[i].color = 0;
	 } else {
	 if (cg_scores_teams) {
	 cg_scores[i].team = i & 1 ? CS_TEAM_GOOD : CS_TEAM_EVIL;
	 cg_scores[i].color = ColorByName(i & 1 ? "blue" : "red", 0);
	 } else {
	 cg_scores[i].team = 0;
	 cg_scores[i].color = (i + 16) * 3;
	 }
	 }
	 }
	 cg_num_scores = i;
	 */

	qsort((void *) cg_scores, cg_num_scores, sizeof(player_score_t), Cg_ParseScores_Compare);
}

/*
 * @brief Returns the vertical screen coordinate where scores should be drawn.
 */
static r_pixel_t Cg_DrawScoresHeader(void) {
	const char *s = cgi.ConfigString(CS_NAME);
	const r_pixel_t sw = cgi.StringWidth(s);
	r_pixel_t cw, ch, x, y;

	cgi.BindFont("medium", &cw, &ch);

	y = cgi.view->y + 64 - ch - 4;

	// map title
	x = cgi.context->width / 2 - sw / 2;
	cgi.DrawString(x, y, s, CON_COLOR_DEFAULT);

	y += ch;

	// team names and scores
	if (cg_scores_teams || cg_scores_ctf) {
		char string[MAX_QPATH];

		player_score_t *score = &cg_scores[cg_num_scores];
		int16_t s = cg_scores_teams ? score->score : score->captures;

		cgi.BindFont("small", &cw, &ch);

		x = cgi.context->width / 2 - SCORES_COL_WIDTH + SCORES_ICON_WIDTH;

		g_snprintf(string, sizeof(string), "%s^7 %d %s",
				cgi.ConfigString(CS_TEAM_GOOD), s,
				cg_scores_ctf ? "caps" : "frags");

		cgi.DrawString(x, y, string, CON_COLOR_BLUE);

		score++;
		s = cg_scores_teams ? score->score : score->captures;

		x += SCORES_COL_WIDTH;

		g_snprintf(string, sizeof(string), "%s^7 %d %s",
				cgi.ConfigString(CS_TEAM_EVIL), s,
				cg_scores_ctf ? "caps" : "frags");

		cgi.DrawString(x, y, string, CON_COLOR_RED);

		y += ch;
	}

	return y;
}

/*
 * @brief
 */
static _Bool Cg_DrawScore(r_pixel_t x, r_pixel_t y, const player_score_t *s) {
	r_pixel_t cw, ch;

	const cl_client_info_t *info = &cgi.client->client_info[s->player_num];

	// icon
	cgi.DrawImage(x, y, 0.33, info->icon);

	//FIXME:
	//if (atoi(cgi.ConfigString(CS_CTF)) && s->flags & SCORES_FLAG)
	//	cgi.DrawPic(x, y, 0.66, "i_quad");

	x += SCORES_ICON_WIDTH;

	// background
	{
		const vec_t fa = s->player_num == cgi.client->player_num ? 0.3 : 0.15;
		const r_pixel_t fw = SCORES_COL_WIDTH - SCORES_ICON_WIDTH - 1;
		const r_pixel_t fh = SCORES_ROW_HEIGHT - 1;

		cgi.DrawFill(x, y, fw, fh, s->color, fa);
	}

	cgi.BindFont("small", &cw, &ch);

	// name
	cgi.DrawString(x, y, info->name, CON_COLOR_DEFAULT);

	// ping
	{
		const r_pixel_t px = x + SCORES_COL_WIDTH - SCORES_ICON_WIDTH - 6 * cw;
		cgi.DrawString(px, y, va("%3dms", s->ping), CON_COLOR_DEFAULT);
		y += ch;
	}

	// spectating
	if (s->team == 0xff) {
		cgi.DrawString(x, y, "spectating", CON_COLOR_DEFAULT);
		return true;
	}

	// frags
	cgi.DrawString(x, y, va("%d frags", s->score), CON_COLOR_DEFAULT);

	// ready/not ready
	if (atoi(cgi.ConfigString(CS_MATCH))) {
		if (s->flags & SCORES_NOT_READY)
			cgi.DrawString(x + cw * 14, y, "not ready", CON_COLOR_DEFAULT);
	}
	y += ch;

	// captures
	if (!cg_scores_ctf)
		return true;

	cgi.DrawString(x, y, va("%d captures", s->captures), CON_COLOR_DEFAULT);
	return true;
}

/*
 * @brief
 */
static void Cg_DrawTeamScores(const r_pixel_t start_y) {
	r_pixel_t x, y;
	int16_t rows;
	size_t i;
	int32_t j = 0;

	rows = (cgi.context->height - (2 * start_y)) / SCORES_ROW_HEIGHT;
	rows = rows < 3 ? 3 : rows;

	x = (cgi.context->width / 2) - SCORES_COL_WIDTH;
	y = start_y;

	for (i = 0; i < cg_num_scores; i++) {
		const player_score_t *s = &cg_scores[i];

		if (s->team != 1)
			continue;

		if ((int16_t) i == rows)
			break;

		if (Cg_DrawScore(x, y, s)) {
			y += SCORES_ROW_HEIGHT;
		}
	}

	x += SCORES_COL_WIDTH;
	y = start_y;

	for (i = 0; i < cg_num_scores; i++) {
		const player_score_t *s = &cg_scores[i];

		if (s->team != 2)
			continue;

		if ((int16_t) i == rows)
			break;

		if (Cg_DrawScore(x, y, s)) {
			y += SCORES_ROW_HEIGHT;
		}
	}

	x -=SCORES_COL_WIDTH;
	y = start_y;

	for (i = 0; i < cg_num_scores; i++) {
		const player_score_t *s = &cg_scores[i];

		if (s->team != 0xFF)
			continue;

		if ((int16_t) i == rows)
			break;

		if (Cg_DrawScore(x, y, s)) {
			if(j++ % 2) {
				x -= SCORES_COL_WIDTH;
				y += SCORES_ROW_HEIGHT;
			}
			else
				x += SCORES_COL_WIDTH;
		}
	}
}

/*
 * @brief
 */
static void Cg_DrawDmScores(const r_pixel_t start_y) {
	int16_t rows, cols;
	r_pixel_t width;
	size_t i;

	rows = (cgi.context->height - (2 * start_y)) / SCORES_ROW_HEIGHT;
	rows = rows < 3 ? 3 : rows;

	cols = (rows < (int16_t) cg_num_scores) ? 2 : 1;
	width = cols * SCORES_COL_WIDTH;

	const player_score_t *s = cg_scores;
	for (i = 0; i < cg_num_scores; i++, s++) {

		if ((int16_t) i == (cols * rows)) // screen is full
			break;

		const int16_t col = i / rows;

		const r_pixel_t x = cgi.context->width / 2 - width / 2 + col * SCORES_COL_WIDTH;
		const r_pixel_t y = start_y + (i % rows) * SCORES_ROW_HEIGHT;

		if (!Cg_DrawScore(x, y, s)) {
			i--;
		}
	}
}

/*
 * @brief
 */
void Cg_DrawScores(const player_state_t *ps) {

	if (!ps->stats[STAT_SCORES])
		return;

	if (!cg_num_scores)
		return;

	const r_pixel_t start_y = Cg_DrawScoresHeader();

	if (cg_scores_teams) {
		Cg_DrawTeamScores(start_y);
	} else if (cg_scores_ctf) {
		Cg_DrawTeamScores(start_y);
	} else {
		Cg_DrawDmScores(start_y);
	}
}
