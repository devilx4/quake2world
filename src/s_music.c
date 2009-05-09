/*
 * Copyright(c) 1997-2001 Id Software, Inc.
 * Copyright(c) 2002 The Quakeforge Project.
 * Copyright(c) 2006 Quake2World.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or(at your option) any later version.
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

#include "client.h"

cvar_t *s_musicvolume;

static s_music_t default_music;


static const char *MUSIC_TYPES[] = {
	".ogg", NULL
};

/**
 * S_LoadMusic
 */
static s_music_t *S_LoadMusic(const char *name){
	void *buf;
	int i, len;
	SDL_RWops *rw;
	static s_music_t music;

	memset(&music, 0, sizeof(music));

	buf = NULL;
	rw = NULL;

	snprintf(music.name, sizeof(music.name), "music/%s", name);

	i = 0;
	while(MUSIC_TYPES[i]){

		Com_StripExtension(music.name, music.name);
		strcat(music.name, MUSIC_TYPES[i++]);

		if((len = Fs_LoadFile(music.name, &buf)) == -1)
			continue;

		if(!(rw = SDL_RWFromMem(buf, len))){
			Fs_FreeFile(buf);
			continue;
		}

		if(!(music.music = Mix_LoadMUS_RW(rw))){
			Com_Warn("S_LoadMusic: %s.\n", Mix_GetError());

			SDL_FreeRW(rw);

			Fs_FreeFile(buf);
			continue;
		}

		music.buffer = buf;

		return &music;
	}

	Com_Warn("S_LoadMusic: Failed to load %s.\n", name);
	return NULL;
}


/*
 * S_FreeMusic
 */
static void S_FreeMusic(s_music_t *music){

	if(!music)
		return;

	if(music->music)
		Mix_FreeMusic(music->music);

	if(music->buffer)
		Fs_FreeFile(music->buffer);
}

/**
 * S_FreeMusics
 */
static void S_FreeMusics(void){
	int i;

	for(i = 0; i < MAX_MUSIC; i++)
		S_FreeMusic(&s_env.music[i]);

	memset(s_env.music, 0, sizeof(s_env.music));
}


/**
 * S_LoadMusics
 */
void S_LoadMusics(void){
	s_music_t *music;
	int i;

	S_FreeMusics();

	for(i = 1; i < MAX_MUSIC; i++){

		if(!cl.configstrings[CS_MUSICS + i][0])
			break;

		if(!(music = S_LoadMusic(cl.configstrings[CS_MUSICS] + i)))
			continue;

		memcpy(&s_env.music[i - 1], music, sizeof(s_music_t));
	}
}


/**
 * S_StopMusic
 */
static void S_StopMusic(void){

	if(s_env.active_music){
		Mix_HaltMusic();

		s_env.active_music = NULL;
	}
}


/**
 * S_FrameMusic
 */
void S_FrameMusic(void){
	s_music_t *music;

	if(s_musicvolume->modified){

		if(s_musicvolume->value > 1.0)
			s_musicvolume->value = 1.0;

		if(s_musicvolume->value < 0.0)
			s_musicvolume->value = 0.0;

		if(s_musicvolume->value)
			Mix_VolumeMusic(s_musicvolume->value * 255);
		else
			S_StopMusic();
	}

	if(!s_musicvolume->value)
		return;

	music = NULL;

	if(cls.state == ca_active){  // play the level-specific music

		if(cl.configstrings[CS_MUSIC][0]){

			const int i = atoi(cl.configstrings[CS_MUSIC]);

			if(i < 0 || i >= MAX_MUSIC){
				Com_Warn("S_FrameMusic: Invalid music index: %d.\n", i);
				return;
			}

			if(s_env.music[i].music)
				music = &s_env.music[i];
		}
	}
	else {  // or play the game theme

		music = &default_music;
	}

	if(music && (music != s_env.active_music)){  // play the new music

		if(s_env.active_music)
			Mix_HaltMusic();

		Mix_PlayMusic(music->music, -1);

		s_env.active_music = music;
	}
}


/**
 * S_InitMusic
 */
void S_InitMusic(void){
	s_music_t *music;

	s_musicvolume = Cvar_Get("s_musicvolume", "0.25", CVAR_ARCHIVE, "Music volume level.");

	S_FreeMusics();

	memset(&default_music, 0, sizeof(default_music));

	music = S_LoadMusic("default");

	if(music)
		memcpy(&default_music, music, sizeof(default_music));
}


/**
 * S_ShutdownMusic
 */
void S_ShutdownMusic(void){

	S_StopMusic();

	S_FreeMusics();

	S_FreeMusic(&default_music);
}