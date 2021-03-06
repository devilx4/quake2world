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

#include "g_local.h"

/*
 * @brief Give items to a client
 */
static void G_Give_f(g_edict_t *ent) {
	const g_item_t *it;
	int32_t index, quantity;
	uint32_t i;
	_Bool give_all;
	g_edict_t *it_ent;

	if (sv_max_clients->integer > 1 && !g_cheats->value) {
		gi.ClientPrint(ent, PRINT_HIGH, "Cheats are disabled\n");
		return;
	}

	const char *name = gi.Args();

	if (gi.Argc() == 3) {
		quantity = atoi(gi.Argv(2));

		if (quantity > 9999)
			quantity = 9999;
	} else
		quantity = 9999;

	if (g_strcmp0(name, "all") == 0)
		give_all = true;
	else
		give_all = false;

	if (give_all || g_strcmp0(gi.Argv(1), "health") == 0) {
		if (gi.Argc() == 3)
			ent->locals.health = quantity;
		else
			ent->locals.health = ent->locals.max_health;
		if (!give_all)
			return;
	}

	if (give_all || g_strcmp0(name, "weapons") == 0) {
		for (i = 0; i < g_num_items; i++) {
			it = g_items + i;
			if (!it->Pickup)
				continue;
			if (it->type != ITEM_WEAPON)
				continue;
			ent->client->locals.persistent.inventory[i] += 1;
		}
		if (!give_all)
			return;
	}

	if (give_all || g_strcmp0(name, "ammo") == 0) {
		for (i = 0; i < g_num_items; i++) {
			it = g_items + i;
			if (!it->Pickup)
				continue;
			if (it->type != ITEM_AMMO)
				continue;
			G_AddAmmo(ent, it, quantity);
		}
		if (!give_all)
			return;
	}

	if (give_all || g_strcmp0(name, "armor") == 0) {
		if (gi.Argc() == 3)
			ent->client->locals.persistent.armor = quantity;
		else
			ent->client->locals.persistent.armor = ent->client->locals.persistent.max_armor;

		if (!give_all)
			return;
	}

	if (give_all) // we've given full health and inventory
		return;

	it = G_FindItem(name);
	if (!it) {
		name = gi.Argv(1);
		it = G_FindItem(name);
		if (!it) {
			gi.ClientPrint(ent, PRINT_HIGH, "Unknown item: %s\n", name);
			return;
		}
	}

	if (!it->Pickup) {
		gi.ClientPrint(ent, PRINT_HIGH, "Non-pickup item: %s\n", name);
		return;
	}

	if (it->type == ITEM_AMMO) { // give the requested ammo quantity
		index = ITEM_INDEX(it);

		if (gi.Argc() == 3)
			ent->client->locals.persistent.inventory[index] = quantity;
		else
			ent->client->locals.persistent.inventory[index] += it->quantity;
	} else { // or spawn and touch whatever they asked for
		it_ent = G_Spawn();
		it_ent->class_name = it->class_name;

		G_SpawnItem(it_ent, it);
		G_TouchItem(it_ent, ent, NULL, NULL);

		if (it_ent->in_use)
			G_FreeEdict(it_ent);
	}
}

/*
 * @brief
 */
static void G_God_f(g_edict_t *ent) {
	char *msg;

	if (sv_max_clients->integer > 1 && !g_cheats->value) {
		gi.ClientPrint(ent, PRINT_HIGH, "Cheats are disabled\n");
		return;
	}

	ent->locals.flags ^= FL_GOD_MODE;
	if (!(ent->locals.flags & FL_GOD_MODE))
		msg = "god OFF\n";
	else
		msg = "god ON\n";

	gi.ClientPrint(ent, PRINT_HIGH, "%s", msg);
}

/*
 * @brief
 */
static void G_Nextmap_f(g_edict_t *ent) {
	gi.AddCommandString(va("map %s\n", G_SelectNextmap()));
}

/*
 * @brief
 */
static void G_NoClip_f(g_edict_t *ent) {
	char *msg;

	if (sv_max_clients->integer > 1 && !g_cheats->value) {
		gi.ClientPrint(ent, PRINT_HIGH, "Cheats are disabled\n");
		return;
	}

	if (ent->locals.move_type == MOVE_TYPE_NO_CLIP) {
		ent->locals.move_type = MOVE_TYPE_WALK;
		msg = "no_clip OFF\n";
	} else {
		ent->locals.move_type = MOVE_TYPE_NO_CLIP;
		msg = "no_clip ON\n";
	}

	gi.ClientPrint(ent, PRINT_HIGH, "%s", msg);
}

/*
 * @brief
 */
static void G_Wave_f(g_edict_t *ent) {

	if (ent->sv_flags & SVF_NO_CLIENT)
		return;

	G_SetAnimation(ent, ANIM_TORSO_GESTURE, true);
}

/*
 * @brief
 */
static void G_Use_f(g_edict_t *ent) {

	if (ent->locals.dead)
		return;

	const char *s = gi.Args();
	const g_item_t *it = G_FindItem(s);
	if (!it) {
		gi.ClientPrint(ent, PRINT_HIGH, "Unknown item: %s\n", s);
		return;
	}
	if (!it->Use) {
		gi.ClientPrint(ent, PRINT_HIGH, "Item is not usable\n");
		return;
	}

	const uint16_t index = ITEM_INDEX(it);
	if (!ent->client->locals.persistent.inventory[index]) {
		gi.ClientPrint(ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->Use(ent, it);
}

/*
 * @brief
 */
static void G_Drop_f(g_edict_t *ent) {
	g_edict_t *f;
	const g_item_t *it;

	// we don't drop in instagib or arena
	if (g_level.gameplay > 1)
		return;

	if (ent->locals.dead)
		return;

	const char *s = gi.Args();
	it = NULL;

	if (!g_strcmp0(s, "flag")) { // find the correct flag

		f = G_FlagForTeam(G_OtherTeam(ent->client->locals.persistent.team));
		if (f)
			it = f->locals.item;
	} else
		// or just look up the item
		it = G_FindItem(s);

	if (!it) {
		gi.ClientPrint(ent, PRINT_HIGH, "Unknown item: %s\n", s);
		return;
	}

	if (!it->Drop) {
		gi.ClientPrint(ent, PRINT_HIGH, "Item is not dropable\n");
		return;
	}

	const uint16_t index = ITEM_INDEX(it);

	if (!ent->client->locals.persistent.inventory[index]) {
		gi.ClientPrint(ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	ent->client->locals.last_dropped = it;
	it->Drop(ent, it);
}

/*
 * @brief
 */
static void G_WeaponPrevious_f(g_edict_t *ent) {
	int32_t i;

	g_client_t *cl = ent->client;

	if (cl->locals.persistent.spectator) {

		if (cl->locals.chase_target) // chase the previous player
			G_ClientChasePrevious(ent);

		return;
	}

	if (!cl->locals.persistent.weapon)
		return;

	const uint16_t selected_weapon = ITEM_INDEX(cl->locals.persistent.weapon);

	// scan for the next valid one
	for (i = 1; i <= MAX_ITEMS; i++) {
		const uint16_t index = (selected_weapon + i) % MAX_ITEMS;

		if (!cl->locals.persistent.inventory[index])
			continue;

		const g_item_t *it = &g_items[index];

		if (!it->Use)
			continue;

		if (it->type != ITEM_WEAPON)
			continue;

		it->Use(ent, it);

		if (cl->locals.persistent.weapon == it)
			return; // successful
	}
}

/*
 * @brief
 */
static void G_WeaponNext_f(g_edict_t *ent) {
	int32_t i;

	g_client_t *cl = ent->client;

	if (cl->locals.persistent.spectator) {

		if (cl->locals.chase_target) // chase the next player
			G_ClientChaseNext(ent);

		return;
	}

	if (!cl->locals.persistent.weapon)
		return;

	const uint16_t selected_weapon = ITEM_INDEX(cl->locals.persistent.weapon);

	// scan  for the next valid one
	for (i = 1; i <= MAX_ITEMS; i++) {
		const uint16_t index = (selected_weapon + MAX_ITEMS - i) % MAX_ITEMS;

		if (!cl->locals.persistent.inventory[index])
			continue;

		const g_item_t *it = &g_items[index];

		if (!it->Use)
			continue;

		if (it->type != ITEM_WEAPON)
			continue;

		it->Use(ent, it);

		if (cl->locals.persistent.weapon == it)
			return; // successful
	}
}

/*
 * @brief
 */
static void G_WeaponLast_f(g_edict_t *ent) {

	g_client_t *cl = ent->client;

	if (!cl->locals.persistent.weapon || !cl->locals.persistent.last_weapon)
		return;

	const uint16_t index = ITEM_INDEX(cl->locals.persistent.last_weapon);

	if (!cl->locals.persistent.inventory[index])
		return;

	const g_item_t *it = &g_items[index];

	if (!it->Use)
		return;

	if (it->type != ITEM_WEAPON)
		return;

	it->Use(ent, it);
}

/*
 * @brief
 */
static void G_Kill_f(g_edict_t *ent) {

	if ((g_level.time - ent->client->locals.respawn_time) < 1000)
		return;

	if (ent->client->locals.persistent.spectator)
		return;

	if (ent->locals.dead)
		return;

	ent->locals.flags &= ~FL_GOD_MODE;
	ent->locals.health = 0;

	means_of_death = MOD_SUICIDE;

	ent->locals.Die(ent, ent, ent, 100000, vec3_origin);
}

/*
 * @brief This is the client-specific sibling to Cvar_VariableString.
 */
static const char *G_ExpandVariable(g_edict_t *ent, char v) {
	int32_t i;

	switch (v) {

		case 'd': // last dropped item
			if (ent->client->locals.last_dropped)
				return ent->client->locals.last_dropped->name;
			return "";

		case 'h': // health
			i = ent->client->ps.stats[STAT_HEALTH];
			return va("%d", i);

		case 'a': // armor
			i = ent->client->ps.stats[STAT_ARMOR];
			return va("%d", i);

		default:
			return "";
	}
}

/*
 * @brief
 */
static char *G_ExpandVariables(g_edict_t *ent, const char *text) {
	static char expanded[MAX_STRING_CHARS];
	int32_t i, j, len;

	if (!text || !text[0])
		return "";

	memset(expanded, 0, sizeof(expanded));
	len = strlen(text);

	for (i = j = 0; i < len; i++) {
		if (text[i] == '%' && i < len - 1) { // expand %variables
			const char *c = G_ExpandVariable(ent, text[i + 1]);
			strcat(expanded, c);
			j += strlen(c);
			i++;
		} else
			// or just append normal chars
			expanded[j++] = text[i];
	}

	return expanded;
}

/*
 * @brief
 */
static void G_Say_f(g_edict_t *ent) {
	char text[MAX_STRING_CHARS];
	char temp[MAX_STRING_CHARS];
	int32_t i;

	g_client_t *cl = ent->client;
	if (cl->locals.muted) {
		gi.ClientPrint(ent, PRINT_HIGH, "You have been muted\n");
		return;
	}

	text[0] = '\0';

	_Bool team = false; // whether or not we're dealing with team chat
	_Bool arg0 = true; // whether or not we need to print arg0

	if (!g_strcmp0(gi.Argv(0), "say") || !g_strcmp0(gi.Argv(0), "say_team")) {
		arg0 = false;

		if (!g_strcmp0(gi.Argv(0), "say_team") && (g_level.teams || g_level.ctf))
			team = true;
	}

	// if g_spectator_chat is off, spectators can only chat to other spectators
	// and so we force team-chat on them
	if (cl->locals.persistent.spectator && !g_spectator_chat->integer) {
		team = true;
	}

	char *s;
	if (arg0) { // not say or say_team, just arbitrary chat from the console
		s = G_ExpandVariables(ent, va("%s %s", gi.Argv(0), gi.Args()));
	} else { // say or say_team
		s = G_ExpandVariables(ent, va("%s", gi.Args()));
	}

	// strip quotes
	if (s[0] == '"' && s[strlen(s) - 1] == '"') {
		s[strlen(s) - 1] = '\0';
		s++;
	}

	// suppress empty messages
	StripColor(s, temp);
	if (!strlen(temp)) {
		return;
	}

	if (!team) { // chat flood protection, does not pertain to teams

		if (g_level.time < cl->locals.chat_time)
			return;

		cl->locals.chat_time = g_level.time + 1000;
	}

	const int32_t color = team ? CON_COLOR_TEAMCHAT : CON_COLOR_CHAT;
	g_snprintf(text, sizeof(text), "%s^%d: %s\n", cl->locals.persistent.net_name, color, s);

	for (i = 1; i <= sv_max_clients->integer; i++) { // print to clients
		const g_edict_t *other = &g_game.edicts[i];

		if (!other->in_use)
			continue;

		if (team) {
			if (!G_OnSameTeam(ent, other))
				continue;
			gi.ClientPrint(other, PRINT_TEAMCHAT, "%s", text);
		} else {
			gi.ClientPrint(other, PRINT_CHAT, "%s", text);
		}
	}

	if (dedicated->value) { // print to the console
		gi.Print("%s", text);
	}
}

/*
 * @brief
 */
static void G_PlayerList_f(g_edict_t *ent) {
	int32_t i, seconds;
	char st[80];
	char text[1400];
	g_edict_t *e2;

	memset(text, 0, sizeof(text));

	// connect time, ping, score, name
	for (i = 0, e2 = g_game.edicts + 1; i < sv_max_clients->integer; i++, e2++) {

		if (!e2->in_use)
			continue;

		seconds = (g_level.frame_num - e2->client->locals.persistent.first_frame) / gi.frame_rate;

		g_snprintf(st, sizeof(st), "%02d:%02d %4d %3d %-16s %s\n", (seconds / 60), (seconds % 60),
				e2->client->ping, e2->client->locals.persistent.score,
				e2->client->locals.persistent.net_name, e2->client->locals.persistent.skin);

		if (strlen(text) + strlen(st) > sizeof(text) - 200) {
			sprintf(text + strlen(text), "And more...\n");
			gi.ClientPrint(ent, PRINT_HIGH, "%s", text);
			return;
		}

		strcat(text, st);
	}
	gi.ClientPrint(ent, PRINT_HIGH, "%s", text);
}

static const char *vote_cmds[] = {
		"g_capture_limit",
		"g_ctf",
		"g_frag_limit",
		"g_friendly_fire",
		"g_gameplay",
		"g_match",
		"g_round_limit",
		"g_rounds",
		"g_spawn_farthest",
		"g_teams",
		"g_time_limit",
		"kick",
		"map",
		"mute",
		"restart",
		"unmute",
		NULL };

/*
 * @brief Inspects the vote command and issues help if applicable. Returns
 * true if the command received help and may therefore be ignored, false
 * otherwise.
 */
static _Bool Vote_Help(g_edict_t *ent) {
	size_t i, j, len;
	char msg[1024];

	if (!g_level.vote_time) { // check for yes/no
		if (gi.Argc() == 1 && (!g_strcmp0(gi.Argv(0), "yes") || !g_strcmp0(gi.Argv(0), "no"))) {
			gi.ClientPrint(ent, PRINT_HIGH, "There is not a vote in progress\n"); // shorthand
			return true;
		}
		if (gi.Argc() == 2 && (!g_strcmp0(gi.Argv(1), "yes") || !g_strcmp0(gi.Argv(1), "no"))) {
			gi.ClientPrint(ent, PRINT_HIGH, "There is not a vote in progress\n"); // explicit
			return true;
		}
	}

	memset(msg, 0, sizeof(msg));

	i = 0;
	if (gi.Argc() == 1) { // no command specified, list them
		strcat(msg, "\nAvailable vote commands:\n\n");

		while (vote_cmds[i]) {
			strcat(msg, "  ");
			strcat(msg, vote_cmds[i]);
			strcat(msg, "\n");
			i++;
		}
		gi.ClientPrint(ent, PRINT_HIGH, "%s", msg);
		return true;
	}

	i = 0;
	while (vote_cmds[i]) { // verify that command is supported
		if (!g_strcmp0(gi.Argv(1), vote_cmds[i]))
			break;
		i++;
	}

	if (!vote_cmds[i]) { // inform client if it is not
		gi.ClientPrint(ent, PRINT_HIGH, "Voting on \"%s\" is not supported\n", gi.Argv(1));
		return true;
	}

	if (!g_strcmp0(gi.Argv(1), "restart"))
		return false; // takes no args, this is okay

	// command-specific help for some commands
	if (gi.Argc() == 2 && !g_strcmp0(gi.Argv(1), "map")) { // list available maps

		if (!g_map_list.count) { // no maps in maplist
			gi.ClientPrint(ent, PRINT_HIGH, "Map voting is not available\n");
			return true;
		}

		strcat(msg, "\nAvailable maps:\n\n");

		j = 0;
		for (i = 0; i < g_map_list.count; i++) {
			len = strlen(g_map_list.maps[i].name) + 3;
			len += strlen(g_map_list.maps[i].title) + 2;

			if (j + len > sizeof(msg)) // don't overrun msg
				break;

			strcat(msg, "  ");
			strcat(msg, g_map_list.maps[i].name);
			strcat(msg, " ");
			strcat(msg, g_map_list.maps[i].title);
			strcat(msg, "\n");
			j += len;
		}

		gi.ClientPrint(ent, PRINT_HIGH, "%s", msg);
		return true;
	}

	if (gi.Argc() == 2 && !g_strcmp0(gi.Argv(1), "g_gameplay")) { // list gameplay modes
		gi.ClientPrint(ent, PRINT_HIGH, "\nAvailable gameplay modes:\n\n"
			"  DEATHMATCH\n  INSTAGIB\n  ARENA\n");
		return true;
	}

	if (gi.Argc() == 2) { // general catch for invalid commands
		gi.ClientPrint(ent, PRINT_HIGH, "Usage: %s <command args>\n", gi.Argv(0));
		return true;
	}

	return false;
}

/*
 * @brief
 */
static void G_Vote_f(g_edict_t *ent) {
	char vote[64];
	uint32_t i;

	if (!g_voting->value) {
		gi.ClientPrint(ent, PRINT_HIGH, "Voting is not allowed");
		return;
	}

	if (!g_strcmp0(gi.Argv(0), "yes") || !g_strcmp0(gi.Argv(0), "no")) // allow shorthand voting
		g_strlcpy(vote, gi.Argv(0), sizeof(vote));
	else { // or the explicit syntax
		g_strlcpy(vote, gi.Args(), sizeof(vote));
	}

	if (g_level.vote_time) { // check for vote from client
		if (ent->client->locals.persistent.vote) {
			gi.ClientPrint(ent, PRINT_HIGH, "You've already voted\n");
			return;
		}
		if (g_strcmp0(vote, "yes") == 0)
			ent->client->locals.persistent.vote = VOTE_YES;
		else if (g_strcmp0(vote, "no") == 0)
			ent->client->locals.persistent.vote = VOTE_NO;
		else { // only yes and no are valid during a vote
			gi.ClientPrint(ent, PRINT_HIGH, "A vote \"%s\" is already in progress\n",
					g_level.vote_cmd);
			return;
		}

		g_level.votes[ent->client->locals.persistent.vote]++;
		gi.BroadcastPrint(PRINT_HIGH, "Voting results \"%s\":\n  %d Yes     %d No\n",
				g_level.vote_cmd, g_level.votes[VOTE_YES], g_level.votes[VOTE_NO]);
		return;
	}

	if (Vote_Help(ent)) // vote command got help, ignore it
		return;

	if (!g_strcmp0(gi.Argv(1), "map")) { // ensure map is in maplist
		for (i = 0; i < g_map_list.count; i++) {
			if (!g_strcmp0(gi.Argv(2), g_map_list.maps[i].name))
				break; // found it
		}

		if (i == g_map_list.count) { // inform client if it is not
			gi.ClientPrint(ent, PRINT_HIGH, "Map \"%s\" is not available\n", gi.Argv(2));
			return;
		}
	}

	g_strlcpy(g_level.vote_cmd, vote, sizeof(g_level.vote_cmd));
	g_level.vote_time = g_level.time;

	ent->client->locals.persistent.vote = VOTE_YES; // client has implicity voted
	g_level.votes[VOTE_YES] = 1;

	gi.ConfigString(CS_VOTE, g_level.vote_cmd); // send to layout

	gi.BroadcastPrint(PRINT_HIGH, "%s has called a vote:\n"
		"  %s\n"
		"To vote, press F1 for yes or F2 for no\n", ent->client->locals.persistent.net_name,
			g_level.vote_cmd);
}

/*
 * @brief Returns true if the client's team was changed, false otherwise.
 */
_Bool G_AddClientToTeam(g_edict_t *ent, const char *team_name) {
	g_team_t *team;

	if (g_level.match_time && g_level.match_time <= g_level.time) {
		gi.ClientPrint(ent, PRINT_HIGH, "Match has already started\n");
		return false;
	}

	if (!(team = G_TeamByName(team_name))) { // resolve team
		gi.ClientPrint(ent, PRINT_HIGH, "Team \"%s\" doesn't exist\n", team_name);
		return false;
	}

	if (ent->client->locals.persistent.team == team)
		return false;

	if (!ent->client->locals.persistent.spectator) { // changing teams
		G_TossQuadDamage(ent);
		G_TossFlag(ent);
	}

	ent->client->locals.persistent.team = team;
	ent->client->locals.persistent.spectator = false;
	ent->client->locals.persistent.ready = false;

	G_ClientUserInfoChanged(ent, ent->client->locals.persistent.user_info);
	return true;
}

/*
 * @brief
 */
static void G_AddClientToRound(g_edict_t *ent) {
	int32_t score; // keep score across rounds

	if (g_level.round_time && g_level.round_time <= g_level.time) {
		gi.ClientPrint(ent, PRINT_HIGH, "Round has already started\n");
		return;
	}

	score = ent->client->locals.persistent.score;

	if (g_level.teams) { // attempt to add client to team
		if (!G_AddClientToTeam(ent, gi.Argv(1)))
			return;
	} else { // simply allow them to join
		if (!ent->client->locals.persistent.spectator)
			return;
		ent->client->locals.persistent.spectator = false;
	}

	G_ClientRespawn(ent, true);
	ent->client->locals.persistent.score = score; // lastly restore score
}

/*
 * @brief
 */
static void G_Team_f(g_edict_t *ent) {

	if ((g_level.teams || g_level.ctf) && gi.Argc() != 2) {
		gi.ClientPrint(ent, PRINT_HIGH, "Usage: %s <%s|%s>\n", gi.Argv(0), g_team_good.name,
				g_team_evil.name);
		return;
	}

	if (g_level.rounds) { // special case for rounds play
		G_AddClientToRound(ent);
		return;
	}

	if (!g_level.teams && !g_level.ctf) {
		gi.ClientPrint(ent, PRINT_HIGH, "Teams are disabled\n");
		return;
	}

	if (!G_AddClientToTeam(ent, gi.Argv(1)))
		return;

	G_ClientRespawn(ent, true);
}

/*
 * @brief
 */
static void G_Teamname_f(g_edict_t *ent) {
	int32_t cs;
	g_team_t *t;

	if (gi.Argc() != 2) {
		gi.ClientPrint(ent, PRINT_HIGH, "Usage: %s <name>\n", gi.Argv(0));
		return;
	}

	if (!ent->client->locals.persistent.team) {
		gi.ClientPrint(ent, PRINT_HIGH, "You're not on a team\n");
		return;
	}

	t = ent->client->locals.persistent.team;

	if (g_level.time - t->name_time < TEAM_CHANGE_TIME)
		return; // prevent change spamming

	const char *s = gi.Argv(1);

	if (*s != '\0') // something valid-ish was provided
		g_strlcpy(t->name, s, sizeof(t->name));
	else
		strcpy(t->name, (t == &g_team_good ? "Good" : "Evil"));

	t->name_time = g_level.time;

	cs = t == &g_team_good ? CS_TEAM_GOOD : CS_TEAM_EVIL;
	gi.ConfigString(cs, t->name);

	gi.BroadcastPrint(PRINT_HIGH, "%s changed team_name to %s\n",
			ent->client->locals.persistent.net_name, t->name);
}

/*
 * @brief
 */
static void G_Teamskin_f(g_edict_t *ent) {
	int32_t i;
	g_client_t *cl;
	g_team_t *t;

	if (gi.Argc() != 2) {
		gi.ClientPrint(ent, PRINT_HIGH, "Usage: %s <skin>\n", gi.Argv(0));
		return;
	}

	if (!ent->client->locals.persistent.team) {
		gi.ClientPrint(ent, PRINT_HIGH, "You're not on a team\n");
		return;
	}

	t = ent->client->locals.persistent.team;

	if (g_level.time - t->skin_time < TEAM_CHANGE_TIME)
		return; // prevent change spamming

	const char *s = gi.Argv(1);

	if (s != '\0') // something valid-ish was provided
		g_strlcpy(t->skin, s, sizeof(t->skin));
	else
		strcpy(t->skin, "qforcer");

	s = t->skin;

	char *c = strchr(s, '/');

	// let players use just the model name, client will find skin
	if (!c || *c == '\0') {
		if (c) // null terminate for strcat
			*c = '\0';

		strncat(t->skin, "/default", sizeof(t->skin) - 1 - strlen(s));
	}

	t->skin_time = g_level.time;

	for (i = 0; i < sv_max_clients->integer; i++) { // update skins
		cl = g_game.clients + i;

		if (!cl->locals.persistent.team || cl->locals.persistent.team != t)
			continue;

		g_strlcpy(cl->locals.persistent.skin, s, sizeof(cl->locals.persistent.skin));

		gi.ConfigString(CS_CLIENTS + i,
				va("%s\\%s", cl->locals.persistent.net_name, cl->locals.persistent.skin));
	}

	gi.BroadcastPrint(PRINT_HIGH, "%s changed team_skin to %s\n",
			ent->client->locals.persistent.net_name, t->skin);
}

/*
 * @brief If match is enabled, all clients must issue ready for game to start.
 */
static void G_Ready_f(g_edict_t *ent) {
	int32_t i, g, e, clients;
	g_client_t *cl;

	if (!g_level.match) {
		gi.ClientPrint(ent, PRINT_HIGH, "Match is disabled\n");
		return;
	}

	if (ent->client->locals.persistent.spectator) {
		gi.ClientPrint(ent, PRINT_HIGH, "You're a spectator\n");
		return;
	}

	if (ent->client->locals.persistent.ready) {
		gi.ClientPrint(ent, PRINT_HIGH, "You're already ready\n");
		return;
	}

	ent->client->locals.persistent.ready = true;

	clients = g = e = 0;

	for (i = 0; i < sv_max_clients->integer; i++) { // is everyone ready?
		cl = g_game.clients + i;

		if (!g_game.edicts[i + 1].in_use)
			continue;

		if (cl->locals.persistent.spectator)
			continue;

		if (!cl->locals.persistent.ready)
			break;

		clients++;

		if (g_level.teams || g_level.ctf)
			cl->locals.persistent.team == &g_team_good ? g++ : e++;
	}

	if (i != (int32_t) sv_max_clients->integer) // someone isn't ready
		return;

	if (clients < 2) // need at least 2 clients to trigger match
		return;

	if ((g_level.teams || g_level.ctf) && (!g || !e)) // need at least 1 player per team
		return;

	if (((int32_t) g_level.teams == 2 || (int32_t) g_level.ctf == 2) && (g != e)) { // balanced teams required
		gi.BroadcastPrint(PRINT_HIGH, "Teams must be balanced for match to start\n");
		return;
	}

	gi.BroadcastPrint(PRINT_HIGH, "Match starting in 10 seconds...\n");
	g_level.match_time = g_level.time + 10000;

	g_level.start_match = true;
}

/*
 * @brief
 */
static void G_Unready_f(g_edict_t *ent) {

	if (!g_level.match) {
		gi.ClientPrint(ent, PRINT_HIGH, "Match is disabled\n");
		return;
	}

	if (ent->client->locals.persistent.spectator) {
		gi.ClientPrint(ent, PRINT_HIGH, "You're a spectator\n");
		return;
	}

	if (g_level.match_time) {
		gi.ClientPrint(ent, PRINT_HIGH, "Match has started\n");
		return;
	}

	if (!ent->client->locals.persistent.ready) {
		gi.ClientPrint(ent, PRINT_HIGH, "You are not ready\n");
		return;
	}

	ent->client->locals.persistent.ready = false;
	g_level.start_match = false;
}

/*
 * @brief
 */
static void G_Spectate_f(g_edict_t *ent) {
	_Bool spectator;

	// prevent spectator spamming
	if (g_level.time - ent->client->locals.respawn_time < 3000)
		return;

	// prevent spectators from joining matches
	if (g_level.match_time && ent->client->locals.persistent.spectator) {
		gi.ClientPrint(ent, PRINT_HIGH, "Match has already started\n");
		return;
	}

	// prevent spectators from joining rounds
	if (g_level.round_time && ent->client->locals.persistent.spectator) {
		gi.ClientPrint(ent, PRINT_HIGH, "Round has already started\n");
		return;
	}

	spectator = ent->client->locals.persistent.spectator;

	if (ent->client->locals.persistent.spectator) { // they wish to join
		if (g_level.teams || g_level.ctf) {
			if (g_auto_join->value) // assign them to a team
				G_AddClientToTeam(ent, G_SmallestTeam()->name);
			else { // or ask them to pick
				gi.ClientPrint(ent, PRINT_HIGH, "Use team <%s|%s> to join the game\n",
						g_team_good.name, g_team_evil.name);
				return;
			}
		}
	} else { // they wish to spectate
		G_TossQuadDamage(ent);
		G_TossFlag(ent);
	}

	ent->client->locals.persistent.spectator = !spectator;
	G_ClientRespawn(ent, true);
}

/*
 * @brief
 */
void G_Score_f(g_edict_t *ent) {
	ent->client->locals.show_scores = !ent->client->locals.show_scores;
}

/*
 * @brief
 */
void G_ClientCommand(g_edict_t *ent) {

	if (!ent->client)
		return; // not fully in game yet

	const char *cmd = gi.Argv(0);

	if (g_strcmp0(cmd, "say") == 0) {
		G_Say_f(ent);
		return;
	}
	if (g_strcmp0(cmd, "say_team") == 0) {
		G_Say_f(ent);
		return;
	}

	// most commands can not be executed during intermission
	if (g_level.intermission_time)
		return;

	if (g_strcmp0(cmd, "score") == 0)
		G_Score_f(ent);
	else if (g_strcmp0(cmd, "spectate") == 0)
		G_Spectate_f(ent);
	else if (g_strcmp0(cmd, "team") == 0 || g_strcmp0(cmd, "join") == 0)
		G_Team_f(ent);
	else if (g_strcmp0(cmd, "team_name") == 0)
		G_Teamname_f(ent);
	else if (g_strcmp0(cmd, "team_skin") == 0)
		G_Teamskin_f(ent);
	else if (g_strcmp0(cmd, "ready") == 0)
		G_Ready_f(ent);
	else if (g_strcmp0(cmd, "unready") == 0)
		G_Unready_f(ent);
	else if (g_strcmp0(cmd, "use") == 0)
		G_Use_f(ent);
	else if (g_strcmp0(cmd, "drop") == 0)
		G_Drop_f(ent);
	else if (g_strcmp0(cmd, "give") == 0)
		G_Give_f(ent);
	else if (g_strcmp0(cmd, "god") == 0)
		G_God_f(ent);
	else if (g_strcmp0(cmd, "no_clip") == 0)
		G_NoClip_f(ent);
	else if (g_strcmp0(cmd, "wave") == 0)
		G_Wave_f(ent);
	else if (g_strcmp0(cmd, "weapon_previous") == 0)
		G_WeaponPrevious_f(ent);
	else if (g_strcmp0(cmd, "weapon_next") == 0)
		G_WeaponNext_f(ent);
	else if (g_strcmp0(cmd, "weapon_last") == 0)
		G_WeaponLast_f(ent);
	else if (g_strcmp0(cmd, "kill") == 0)
		G_Kill_f(ent);
	else if (g_strcmp0(cmd, "player_list") == 0)
		G_PlayerList_f(ent);
	else if (g_strcmp0(cmd, "vote") == 0)
		G_Vote_f(ent);
	else if (g_strcmp0(cmd, "yes") == 0 || g_strcmp0(cmd, "no") == 0)
		G_Vote_f(ent);
	else
		// anything that doesn't match a command will be a chat
		G_Say_f(ent);
}
