AC_PREREQ(2.56)

AC_INIT(quake2world, 0.1.0, jay@jaydolan.com)
AC_GNU_SOURCE

AC_CONFIG_AUX_DIR(.)
AC_CONFIG_MACRO_DIR(m4)
AC_CONFIG_SRCDIR(src/shared.c)
AC_CONFIG_HEADERS(config.h)

AC_CANONICAL_SYSTEM
AC_DEFINE_UNQUOTED(BUILD_HOST,
	"${target_cpu}-${target_os}",
	[Set to the canonical name of the target machine]
)

AC_SUBST(PROGRAM)
AC_SUBST(VERSION)

AM_INIT_AUTOMAKE([silent-rules foreign dist-bzip2 dist-zip])
AM_SILENT_RULES([yes])

AM_MAINTAINER_MODE

dnl ---------------
dnl Check for tools
dnl ---------------

AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL

dnl ---------------
dnl Check host type
dnl ---------------

HOST_NAME=""
HOST_CPPFLAGS=""
HOST_CXXFLAGS=""
HOST_CFLAGS=""
HOST_LDFLAGS=""
HOST_LIBS=""
HOST_DEBUG_CFLAGS=""
HOST_DEBUG_LIBS=""
HOST_PROFILE_CFLAGS=""
HOST_PROFILE_LIBS=""
HOST_SO_CFLAGS=""
DEFAULT_PREFIX="/usr/local"

AC_MSG_CHECKING([host type])
case "$host" in
	*-apple-*)
		HOST_NAME="APPLE"
		HOST_CPPFLAGS="-I/opt/local/include"
		HOST_CXXFLAGS="-I/opt/local/include"
		HOST_CFLAGS="-I/opt/local/include"
		HOST_LDFLAGS="-L/opt/local/lib -headerpad_max_install_names"
		HOST_LIBS="-ldl"
		HOST_DEBUG_LIBS="-rdynamic"
		HOST_SO_CFLAGS="-fPIC -DPIC"
		AC_MSG_RESULT(apple)
		;;
	*bsd*)
		HOST_NAME="BSD"
		HOST_SO_CFLAGS="-fPIC -DPIC"
		AC_MSG_RESULT(bsd)
		;;
	*-*-mingw*)
		HOST_NAME="WINDOWS"
		HOST_LIBS="-lws2_32 -lole32 -lstdc++"
		DEFAULT_PREFIX="/tmp/quake2world-$target_os"
		AC_PATH_PROGS([WINDRES], [windres $target_cpu-$target_os-windres $host_alias-windres $host_os-windres i586-mingw32msvc-windres])
		if test "x$WINDRES" = "x"; then
			AC_MSG_ERROR([Could not find windres, please install windres])
		fi
		AC_SUBST(WINDRES)
		AC_MSG_RESULT(windows)
		;;
	*)
		HOST_NAME="LINUX"
		HOST_LIBS="-ldl -lm -lstdc++"
		HOST_DEBUG_LIBS="-rdynamic"
		HOST_SO_CFLAGS="-fPIC -DPIC"
		AC_MSG_RESULT(linux)
		;;
esac

AC_SUBST(HOST_NAME)
AC_SUBST(HOST_CPPFLAGS)
AC_SUBST(HOST_CXXFLAGS)
AC_SUBST(HOST_CFLAGS)
AC_SUBST(HOST_LIBS)
AC_SUBST(HOST_DEBUG_CFLAGS)
AC_SUBST(HOST_DEBUG_LIBS)
AC_SUBST(HOST_PROFILE_CFLAGS)
AC_SUBST(HOST_PROFILE_LIBS)

CPPFLAGS="$CPPFLAGS $HOST_CPPFLAGS"
AC_SUBST(CPPFLAGS)

CXXFLAGS="-pipe $CXXFLAGS $HOST_CXXFLAGS"
AC_SUBST(CXXFLAGS)

CFLAGS="-std=c99 -pipe $CFLAGS $HOST_CFLAGS"
AC_SUBST(CFLAGS)

SO_CFLAGS="$SO_CFLAGS $HOST_SO_CFLAGS"
AC_SUBST(SO_CFLAGS)

LDFLAGS="$LDFLAGS $HOST_LDFLAGS"
AC_SUBST(LDFLAGS)

AM_CONDITIONAL([APPLE], [test "x$HOST_NAME" = "xAPPLE"])
AM_CONDITIONAL([BSD], [test "x$HOST_NAME" = "xBSD"])
AM_CONDITIONAL([WINDOWS], [test "x$HOST_NAME" = "xWINDOWS"])
AM_CONDITIONAL([LINUX], [test "x$HOST_NAME" = "xLINUX"])

dnl --------------
dnl Check for GLib
dnl --------------
PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.0.0])

dnl ----------------
dnl Check for PHYSFS
dnl ----------------

AC_ARG_WITH(physfs,
	AS_HELP_STRING([--with-physfs=DIR],
		[use alternate location of PhysFS]
	)
)

if test "x${with_physfs}" != x; then
	PHYSFS_CFLAGS="$PHYSFS_CFLAGS -I${with_physfs}/include"
	PHYSFS_LIBS="$PHYSFS_LDFLAGS -L${with_physfs}/lib"
fi

save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $PHYSFS_CFLAGS"
	
AC_CHECK_HEADER(physfs.h,
	PHYSFS_LIBS="$PHYSFS_LIBS -lphysfs",
	[AC_MSG_ERROR([Could not find physfs.h, please install PhysFS])]
)
AC_SUBST(PHYSFS_CFLAGS)
AC_SUBST(PHYSFS_LIBS)

CPPFLAGS="$save_CPPFLAGS"

dnl -------------
dnl Check for SDL
dnl -------------

AC_PATH_PROG(SDL_CONFIG, sdl-config,
	[AC_MSG_ERROR([Could not find sdl-config, please install libsdl])]
)

SDL_CFLAGS=`$SDL_CONFIG --cflags`
SDL_LIBS=`$SDL_CONFIG --libs`

dnl -----------------------------
dnl Check whether to build client
dnl -----------------------------

BUILD_CLIENT=no

AC_ARG_WITH(client,
	AS_HELP_STRING([--without-client], [build a dedicated server])
)

AC_MSG_CHECKING(whether to build client)

if test "x${with_client}" = xno; then
    AC_MSG_RESULT(no)
else
	AC_MSG_RESULT(yes)
	BUILD_CLIENT=yes

	AC_DEFINE(BUILD_CLIENT, 1, [Define to 1 to build a client.])
	
	dnl ------------------------------------------
	dnl Check for SDL_opengl, SDL_image, SDL_mixer
	dnl ------------------------------------------

	save_CPPFLAGS="$CPPFLAGS"
	CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"
	
	AC_CHECK_HEADER(SDL/SDL_opengl.h,,
		[AC_MSG_ERROR([Could not find SDL_opengl.h, please install libsdl])]
	)

	AC_CHECK_HEADER(SDL/SDL_image.h,
		SDL_LIBS="$SDL_LIBS -lSDL_image",
		[AC_MSG_ERROR([Could not find SDL_image.h, please install libsdl-image])]
	)

	AC_CHECK_HEADER(SDL/SDL_mixer.h,
		SDL_LIBS="$SDL_LIBS -lSDL_mixer",
		[AC_MSG_ERROR([Could not find SDL_mixer.h, please install libsdl-mixer])]
	)
	
	CPPFLAGS="$save_CPPFLAGS"
	
	dnl -----------------------------------
	dnl Sort out OpenGL an flags and libraries
	dnl -----------------------------------
	
	HOST_OPENGL_CFLAGS=""
	HOST_OPENGL_LIBS=""
	
	case "$host" in
		*-apple-*)
			HOST_OPENGL_LIBS="-framework OpenGL"
			;;
		*bsd*)
			HOST_OPENGL_LIBS="-lGL"
			;;
		*-*-mingw*)
			HOST_OPENGL_LIBS="-lopengl32"
			;;
		*)
			HOST_OPENGL_LIBS="-lGL"
			;;
	esac
	
	AC_SUBST(HOST_OPENGL_CFLAGS)
	AC_SUBST(HOST_OPENGL_LIBS)

	OPENGL_CFLAGS="$OPENGL_CFLAGS $HOST_OPENGL_CFLAGS"
	AC_SUBST(OPENGL_CFLAGS)
	
	OPENGL_LIBS="$OPENGL_LIBS $HOST_OPENGL_LIBS"
	AC_SUBST(OPENGL_LIBS)
	
	dnl ----------------------------------------
	dnl Sort out AntTweakBar flags and libraries
	dnl ----------------------------------------
	
	HOST_ANTTWEAKBAR_CXXFLAGS=""
	HOST_ANTTWEAKBAR_CFLAGS=""
	HOST_ANTTWEAKBAR_LIBS=""
	
	case "$host" in
		*-apple-*)
			HOST_ANTTWEAKBAR_CXXFLAGS="-D_MACOSX -I/usr/X11R6/include -ObjC++"
			HOST_ANTTWEAKBAR_CFLAGS="-D_MACOSX -I/usr/X11R6/include"
			HOST_ANTTWEAKBAR_LIBS="-framework GLUT -framework AppKit"
			;;
		*-*-mingw*)
			HOST_ANTTWEAKBAR_CXXFLAGS="-D_WINDOWS -fpermissive"
			HOST_ANTTWEAKBAR_CFLAGS="-D_WINDOWS -fpermissive"
			;;
		*)
			HOST_ANTTWEAKBAR_CXXFLAGS="-D_UNIX -I/usr/X11R6/include"
			HOST_ANTTWEAKBAR_CFLAGS="-D_UNIX -I/usr/X11R6/include"
			HOST_ANTTWEAKBAR_LIBS="-lX11 -lXext"
			;;
	esac
	
	AC_SUBST(HOST_ANTTWEAKBAR_CXXFLAGS)
	AC_SUBST(HOST_ANTTWEAKBAR_CFLAGS)
	AC_SUBST(HOST_ANTTWEAKBAR_LIBS)
	
	ANTTWEAKBAR_CXXFLAGS="$ANTTWEAKBAR_CXXFLAGS $HOST_ANTTWEAKBAR_CXXFLAGS"
	AC_SUBST(ANTTWEAKBAR_CXXFLAGS)

	ANTTWEAKBAR_CFLAGS="$ANTTWEAKBAR_CFLAGS $HOST_ANTTWEAKBAR_CFLAGS"
	AC_SUBST(ANTTWEAKBAR_CFLAGS)
	
	ANTTWEAKBAR_LIBS="$ANTTWEAKBAR_LIBS $HOST_ANTTWEAKBAR_LIBS"
	AC_SUBST(ANTTWEAKBAR_LIBS)
	
	dnl --------------
	dnl Check for cURL
	dnl --------------

	AC_PATH_PROG(CURL_CONFIG, curl-config)

	if test "x${CURL_CONFIG}" != x; then
		CURL_CFLAGS=`$CURL_CONFIG --cflags`
		CURL_LIBS=`$CURL_CONFIG --libs`
	else
		AC_MSG_ERROR([Could not find curl-config, please install cURL])
	fi
	AC_SUBST(CURL_CFLAGS)
	AC_SUBST(CURL_LIBS)

	dnl --------------
	dnl Check for JPEG
	dnl --------------

	AC_CHECK_HEADER(jpeglib.h,
		JPEG_LIBS="-ljpeg",
		[AC_MSG_ERROR([Could not find jpeglib.h, please install libjpeg])]
	)
	AC_SUBST(JPEG_LIBS)

	dnl --------------------------
	dnl Check which tools to build
	dnl --------------------------

	AC_MSG_CHECKING(which tools to build)

	TOOLS="q2wmap"
	TOOL_DIRS="$TOOLS"
	
	AC_ARG_WITH(tools,
		AS_HELP_STRING([--with-tools='q2wmap ...'],
			[build specified tools]
		)
	)

	if test "x${with_tools}" = xno; then
		TOOLS="no"
		TOOL_DIRS=""
	elif test "x${with_tools}" != x; then
		TOOLS="${with_tools}"
		TOOL_DIRS="${with_tools}"
	fi
	
	AC_MSG_RESULT($TOOLS)
	AC_SUBST(TOOL_DIRS)
	
fi

AM_CONDITIONAL([BUILD_CLIENT], [test "x$BUILD_CLIENT" = "xyes"])
AM_CONDITIONAL([BUILD_TOOLS], [test "x$TOOL_DIRS" != "x"])

AC_SUBST(SDL_CFLAGS)
AC_SUBST(SDL_LIBS)

dnl -------------------------------
dnl Check for stdint.h
dnl -------------------------------
AC_CHECK_HEADERS([stdint.h])

dnl -------------------------------
dnl Check for execinfo.h (optional)
dnl -------------------------------

AC_CHECK_HEADER(execinfo.h,
	AC_DEFINE(HAVE_EXECINFO, 1,
	[Define to 1 if you have the <execinfo.h> header file.]),
)

dnl ---------------------------
dnl Check for curses (optional)
dnl ---------------------------

HAVE_CURSES=no

AC_ARG_WITH(curses,
	AS_HELP_STRING([--with-curses(=DIR)],
		[use curses (optionally, found in DIR)]
	)
)

if test "x${with_curses}" != xno; then
	if test "x${with_curses}" != xyes; then
		CURSES_CFLAGS="$CURSES_CFLAGS -I${with_curses}/include"
		CURSES_LDFLAGS="$CURSES_LDFLAGS -L${with_curses}/lib"
	fi
	save_CPPFLAGS="$CPPFLAGS"
	save_LDFLAGS="$LDFLAGS"

	CPPFLAGS="$CPPFLAGS $CURSES_CFLAGS"
	LDFLAGS="$LDFLAGS $CURSES_LDFLAGS"

	dnl -----------------------
	dnl Check for curses header
	dnl -----------------------

	AC_CHECK_HEADER(curses.h,
		HAVE_CURSES=maybe,
		HAVE_CURSES=no
	)

	dnl -----------------
	dnl Check for ncurses
	dnl -----------------

	if test "${HAVE_CURSES}" = maybe; then
		AC_CHECK_LIB(ncurses, initscr,
			HAVE_CURSES=ncurses
			AC_DEFINE(HAVE_CURSES, "ncurses",
					[Define to value if you have a curses compatible library.])
			CURSES_LIBS="$CURSES_LIBS -lncurses",
			HAVE_CURSES=maybe
		)
	fi

	dnl ------------------
	dnl Check for pdcurses
	dnl ------------------

	if test "${HAVE_CURSES}" = maybe; then
		AC_CHECK_LIB(pdcurses, initscr,
			HAVE_CURSES=pdcurses
			AC_DEFINE(HAVE_CURSES, "pdcurses",
					[Define to value if you have a curses compatible library.])
			CURSES_LIBS="$CURSES_LIBS -lpdcurses",
			HAVE_CURSES=maybe
		)
	fi

	dnl ------------------------
	dnl Check for generic curses
	dnl ------------------------

	if test "${HAVE_CURSES}" = maybe; then
		AC_CHECK_LIB(curses, initscr,
			HAVE_CURSES=curses
			AC_DEFINE(HAVE_CURSES, "curses",
					[Define to value if you have a curses library.])
			CURSES_LIBS="$CURSES_LIBS -lcurses",
			HAVE_CURSES=no
		)
	fi
		
	dnl --------------------------------------------
	dnl For dedicated servers, curses is recommended
	dnl --------------------------------------------
	
	if test "${HAVE_CURSES}" = no -a "${with_client}" = no; then
		AC_MSG_ERROR([Could not find curses, which is strongly recommended for dedicated servers.
		You should install curses, or ./configure --without-curses to ignore this error.
		])
	fi

	CPPFLAGS="$save_CPPFLAGS"
	LDFLAGS="$save_LDFLAGS"
fi

AC_SUBST(CURSES_CFLAGS)
AC_SUBST(CURSES_LDFLAGS)
AC_SUBST(CURSES_LIBS)

dnl --------------------------
dnl Check for MySQL (optional)
dnl --------------------------

HAVE_MYSQL=no

AC_ARG_WITH(mysql,
	AS_HELP_STRING([--with-mysql],
		[use mysql (relies on mysql_config)]
	)
)

AC_PATH_PROG(MYSQL_CONFIG, mysql_config)

if test "x${with_mysql}" != xno -a "x${MYSQL_CONFIG}" != x; then
	MYSQL_CFLAGS=`$MYSQL_CONFIG --cflags`

	save_CPPFLAGS="$CPPFLAGS"
	CPPFLAGS="$CPPFLAGS $MYSQL_CFLAGS"

	AC_CHECK_HEADER(mysql.h,
		HAVE_MYSQL=yes
		AC_DEFINE(HAVE_MYSQL, 1,
				[Define to 1 if you have the mysql.h header file.])
		MYSQL_LIBS=`$MYSQL_CONFIG --libs`,
		HAVE_MYSQL=no
	)

	CPPFLAGS="$save_CPPFLAGS"
fi
AC_SUBST(MYSQL_CFLAGS)
AC_SUBST(MYSQL_LIBS)

dnl ---------------------------------
dnl Check which game modules to build
dnl ---------------------------------

AC_MSG_CHECKING(which game modules to build)

GAME_MODULES="default"
AC_ARG_WITH(games,
    AS_HELP_STRING([--with-games='default ...'],
		[build specified game modules]
	)
)

if test "x${with_games}" != x; then
    GAME_MODULES="${with_games}"
fi
AC_SUBST(GAME_MODULES)
AC_MSG_RESULT($GAME_MODULES)

dnl ------------------------------------
dnl Check whether to build master server
dnl ------------------------------------

BUILD_MASTER=no

AC_ARG_WITH(master,
	AS_HELP_STRING([--with-master],
		[build a master server]
	)
)

AC_MSG_CHECKING(whether to build master server)

if test "x${with_master}" = xyes; then
	BUILD_MASTER=yes
	AC_DEFINE(BUILD_MASTER, 1, [Define to 1 to build a master server.])
fi
AC_MSG_RESULT($BUILD_MASTER)

dnl ---------------------------------
dnl Check whether to build unit tests
dnl ---------------------------------

BUILD_TESTS=no

AC_ARG_WITH(tests,
	AS_HELP_STRING([--with-tests],
		[build unit tests]
	)
)

AC_MSG_CHECKING(whether to build unit tests)

if test "x${with_tests}" = xyes; then
	BUILD_TESTS=yes
	AC_MSG_RESULT($BUILD_TESTS)
	PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])
fi

AM_CONDITIONAL(BUILD_TESTS, test "x$BUILD_TESTS" = xyes)

dnl -----------------
dnl Compiler warnings
dnl -----------------

AC_MSG_CHECKING(whether to abort on compiler warnings)

WARN_CFLAGS="-pedantic -W -Wall"

AC_ARG_ENABLE(warn,
	AS_HELP_STRING(
		[--enable-warn], [abort on compiler warnings]
	),
	AC_MSG_RESULT(yes)
	WARN_CFLAGS="-Werror $WARN_CFLAGS",
	AC_MSG_RESULT(no)
)
AC_SUBST(WARN_CFLAGS)

dnl ----------------------
dnl Compiler optimizations
dnl ----------------------

AC_MSG_CHECKING(whether to enable compiler optimizations)

OPT_CFLAGS=""

AC_ARG_ENABLE(optimizations,
	AS_HELP_STRING(
		[--enable-optimizations=none|default|expensive], [enable compiler optimizations]
	),
	AC_MSG_RESULT($enable_optimizations)
	if test "x${enable_optimizations}" = xnone; then
		OPT_CFLAGS="-O0 $OPT_CFLAGS"
	elif test "x${enable_optimizations}" = xexpensive; then
		OPT_CFLAGS="-O3 $OPT_CFLAGS"
	fi,
	AC_MSG_RESULT(default)
)
AC_SUBST(OPT_CFLAGS)

dnl ---------------------
dnl Debugging information
dnl ---------------------

AC_MSG_CHECKING(whether to include debugging information)

DEBUG_CFLAGS=""
DEBUG_LIBS=""

AC_ARG_ENABLE(debug,
	AS_HELP_STRING(
		[--enable-debug], [include debugging information]
	),
	AC_MSG_RESULT(yes)
	DEBUG_CFLAGS="-g $DEBUG_CFLAGS $HOST_DEBUG_CFLAGS"
	DEBUG_LIBS="$DEBUG_LIBS $HOST_DEBUG_LIBS",
	AC_MSG_RESULT(no)
)
AC_SUBST(DEBUG_CFLAGS)
AC_SUBST(DEBUG_LIBS)

dnl ---------------------
dnl Profiling information
dnl ---------------------

AC_MSG_CHECKING(whether to include profiling information)

PROFILE_CFLAGS=""
PROFILE_LIBS=""

AC_ARG_ENABLE(profile,
	AS_HELP_STRING(
		[--enable-profile], [include profiling information]
	),
	AC_MSG_RESULT(yes)
	PROFILE_CFLAGS="-pg $HOST_PROFILE_CFLAGS"
	PROFILE_LIBS="$HOST_PROFILE_LIBS",
	AC_MSG_RESULT(no)
)
AC_SUBST(PROFILE_CFLAGS)
AC_SUBST(PROFILE_LIBS)

dnl ----------------------------------------
dnl Consolidate our core flags and libraries
dnl ----------------------------------------

BASE_CPPFLAGS=$(echo $CPPFLAGS)
BASE_CXXFLAGS=$(echo $CXXFLAGS $OPT_CFLAGS $DEBUG_CFLAGS $PROFILE_CFLAGS $WARN_CFLAGS)
BASE_CFLAGS=$(echo $CFLAGS $OPT_CFLAGS $DEBUG_CFLAGS $PROFILE_CFLAGS $WARN_CFLAGS)
BASE_LIBS=$(echo $HOST_LIBS $DEBUG_LIBS $PROFILE_LIBS $WARN_LIBS)
BASE_LDFLAGS=$(echo $LDFLAGS)

AC_SUBST(BASE_CPPFLAGS)
AC_SUBST(BASE_CXXFLAGS)
AC_SUBST(BASE_CFLAGS)
AC_SUBST(BASE_LIBS)
AC_SUBST(BASE_LDFLAGS)

dnl ----------------------
dnl Fill in path variables
dnl ----------------------
test "$prefix" = "NONE" && prefix=$DEFAULT_PREFIX
test "$exec_prefix" = "NONE" && exec_prefix=$prefix

if test "x${HOST_NAME}" = "xWINDOWS"; then
	eval BINDIR="$bindir"
	eval PKGLIBDIR="$prefix/lib"
	eval PKGDATADIR="$prefix/share"
else
	eval BINDIR="$bindir"
	eval PKGLIBDIR="$prefix/lib/$PACKAGE"
	eval PKGDATADIR="$prefix/share/$PACKAGE"
fi

AC_DEFINE_UNQUOTED(PKGLIBDIR, "$PKGLIBDIR",
	[Define to path containing the shared modules.]
)

AC_DEFINE_UNQUOTED(PKGDATADIR, "$PKGDATADIR",
	[Define to path containing the game data.]
)

AC_SUBST(PKGDATADIR)

AM_CONDITIONAL(BUILD_CLIENT, test "x$BUILD_CLIENT" = xyes)
AM_CONDITIONAL(BUILD_MASTER, test "x$BUILD_MASTER" = xyes)

dnl -----------------------------------------------------------
dnl Configuraton tests complete -- provide a summary of results
dnl -----------------------------------------------------------

AC_CONFIG_FILES([
	Makefile
	deps/Makefile
	deps/AntTweakBar/Makefile
	deps/AntTweakBar/include/Makefile
	deps/AntTweakBar/src/Makefile
	deps/minizip/Makefile
	doc/Makefile
	src/Makefile
	src/ai/Makefile
	src/cgame/Makefile
	src/cgame/default/Makefile
	src/client/Makefile
	src/client/renderer/Makefile
	src/client/renderer/shaders/Makefile
	src/client/sound/Makefile
	src/client/ui/Makefile
	src/game/Makefile
	src/game/default/Makefile
	src/main/Makefile
	src/master/Makefile
	src/server/Makefile
	src/tests/Makefile
	src/tools/Makefile
	src/tools/q2wmap/Makefile
])

AC_OUTPUT

AC_MSG_RESULT([
Quake2World $VERSION: Automatic configuration OK.

  Configuration summary:
    Platform: .......... $host
    Compilers: ......... $CC, $CXX
    Preprocessor flags.. $BASE_CPPFLAGS
    C++ flags: ......... $BASE_CXXFLAGS
    C flags: ........... $BASE_CFLAGS
    Libraries: ......... $BASE_LIBS
    Linking: ........... $BASE_LDFLAGS

    Client: ............ $BUILD_CLIENT
    Game modules: ...... $GAME_MODULES
    Tools: ............. ${TOOLS:-None}
    Master server: ..... $BUILD_MASTER
    Unit tests: ........ $BUILD_TESTS

    Options:
      curses: .......... $HAVE_CURSES
      MySQL: ........... $HAVE_MYSQL

  Installation directories:

    Program: ........... $BINDIR
    Libraries: ......... $PKGLIBDIR
    Data: .............. $PKGDATADIR

Type 'make' to build Quake2World.  Type 'make install' to install it.
Type 'make rsync-data' to install or update the game data.
])
