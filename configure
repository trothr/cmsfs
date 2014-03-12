#!/bin/sh
#
#
#         Name: CMSFSCFG SH (shell script)
#               should be copied to  'configure'  and executed
#               Adds leading TAB characters required by 'make'
#               and performs other substitutions based on platform.
#       Author: Rick Troth, BMC Software, Inc., Houston, Texas, USA
#         Date: 2000-Nov-03 (Fri)
#		2002-Nov-08 (Fri)
#
#

#
# char set testing ^ ! ~ @ $ % & * [] {} `' \/ <> () "
# char set testing N E T A D P A A OC OC OC BF OC OC Q
# char set testing O X L T O C M S BB BB QQ SS AA PP O
# char set testing T C D   L T P T KK RR TT LL NN RR T
#

#
#
if [ -z "$MAKE" ] ; then MAKE=make ; export MAKE ; fi

#
# be sure all shell scripts are executable
chmod +x *.sh ; RC="$?"
if [ "$RC" != 0 ] ; then exit $RC ; fi

#
# report
echo "***"
echo "*** UNAME says this is a `uname` system."

#
# stage configuration into a SED script
chmod +x cmsfssed.sh
rm -f cmsfscfg.sed
./cmsfssed.sh $* > cmsfscfg.sed ; RC="$?"
if [ "$RC" != 0 ] ; then exit $RC ; fi

#
# retrieve substitutions from the 'sed' operation
sh -c ' echo "PREFIX=%PREFIX%" ; \
	echo "MODULES_DIRECTORY=%MODULES_DIRECTORY%" ; \
	echo "DRIVER_SOURCE=%DRIVER_SOURCE%"' \
	| sed -f cmsfscfg.sed > cmsfscfg.tmp
# s#%DEFINES%#-DCMSFS_HOST_ASCII#g
# s#%INCLUDES%##g
# s#%LINUX_RELEASE%#2.2#g
. ./cmsfscfg.tmp
rm cmsfscfg.tmp

#
# report
if [ ! -z "$PREFIX" ] ; then
	echo "*** will install utilities to '$PREFIX'" ; fi
if [ ! -z "$MODULES_DIRECTORY" ] ; then
	echo "*** will install module to '$MODULES_DIRECTORY'" ; fi
if [ ! -z "$DRIVER_SOURCE" ] ; then
	echo "*** using driver source '$DRIVER_SOURCE' for VFS" ; fi

#
# report
echo "***"

#
# run the makefile source through 'sed'
if [ -f makefile ] ; then mv makefile makefile-OLD ; fi
sed -f cmsfscfg.sed < cmsfs.makefile > makefile ; RC="$?"
if [ "$RC" != 0 ] ; then exit $RC ; fi
# and refresh the 'configure' script based on that
$MAKE configure > /dev/null ; RC="$?"
if [ "$RC" != 0 ] ; then exit $RC ; fi

#
# be sure we're set-up for the right driver
rm -f cmsfsvfs.c
$MAKE cmsfsvfs.c





