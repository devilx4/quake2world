#!/bin/sh
#############################################################################
# Copyright(c) 2007-2012 Quake2World.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 3
# of the License, or(at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#############################################################################



if [ ! -e /mingw/bin/gcc.exe ]; then
  echo "/mingw is not mounted or gcc not installed"
  exit 1
fi


function BUILD
{
	CURRENT_ARCH=`gcc -v 2>&1|grep Target|cut -d\  -f2|cut -d\- -f1`
	rm -f _build-$CURRENT_ARCH.log
	
	gcc -v >> _build-"$CURRENT_ARCH".log 2>&1
	sh _build_win32.sh >> _build-"$CURRENT_ARCH".log 2>&1
	
	if [ $? != "0" ];then
		echo "Build error"
		sync
    	./_rsync_retry.sh -vrzhP --timeout=120 --delete --inplace --rsh='ssh'  _build-"$CURRENT_ARCH".log maci@quake2world.net:/var/www/quake2world/files/
		mailsend.exe -d quake2world.net -smtp quake2world.net -t quake2world-dev@quake2world.net -f q2wbuild@quake2world.net -sub "Build FAILED $CURRENT_ARCH-svn$CURRENT_REVISION" +cc +bc -a "_build-$CURRENT_ARCH.log,text/plain"
	else
		echo "Build succeeded"
		sync
    	./_rsync_retry.sh -vrzhP --timeout=120 --delete --inplace --rsh='ssh'  _build-"$CURRENT_ARCH".log maci@quake2world.net:/var/www/quake2world/files/
		rm _build-"$CURRENT_ARCH".log
	fi
	
}

while true; do
	CURRENT_ARCH=`gcc -v 2>&1|grep Target|cut -d\  -f2|cut -d\- -f1`
	CURRENT_REVISION=`svn info svn://quake2world.net/quake2world|grep Revision:|cut -d\  -f2`
	
	
	if [ -e $CURRENT_ARCH/quake2world ]; then
		LAST_REVISION=`svn info	$CURRENT_ARCH/quake2world|grep Revision:|cut -d\  -f2`
	else
		LAST_REVISION="0"
	fi
		
	echo $LAST_REVISION $CURRENT_REVISION

	if [ -e _build-i686.log -o -e _build-x86_64.log -o $LAST_REVISION != $CURRENT_REVISION ]; then
		echo "Building......" - `date`
		BUILD
		sh ../switch_arch.sh
		BUILD
		sh ../switch_arch.sh
		echo "Building done." - `date`
	else
		echo "No new revision and no previous failed build." - `date`
	fi

	sleep 3h
	
done
