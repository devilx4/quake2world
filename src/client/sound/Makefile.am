noinst_HEADERS = \
	s_local.h \
	s_main.h \
	s_media.h \
	s_mix.h \
	s_music.h \
	s_sample.h \
	s_types.h \
	sound.h

noinst_LTLIBRARIES = \
	libsound.la

if BUILD_CLIENT

libsound_la_SOURCES = \
	s_main.c \
	s_media.c \
	s_mix.c \
	s_music.c \
	s_sample.c

libsound_la_CFLAGS = \
	-I.. -I../.. \
	@BASE_CFLAGS@ \
	@GLIB_CFLAGS@ \
	@SDL_CFLAGS@

libsound_la_LIBADD = \
	../../libshared.la \
	@SDL_LIBS@

else

libsound_la_SOURCES = \
	../../null.c
	
libsound_la_CFLAGS = \
	-D__LIBTOOL_LIBRARY__=libsound_la

endif