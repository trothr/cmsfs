#!/bin/sh
#
#
#         Name: CMSFSSED SH (shell script)
#               should be copied to  'configure'  and executed
#               Adds leading TAB characters required by 'make'
#               and performs other substitutions based on platform.
#       Author: Rick Troth, BMC Software, Inc., Houston, Texas, USA
#         Date: 2000-Nov-03 (Fri)
#               2002-Nov-04 (Mon)
#               2002-Nov-08 (Fri)
#
#
#

Z=`basename "$0"`
#
# char set testing ^ ! ~ @ $ % & * [] {} `' \/ <> () "
# char set testing N E T A D P A A OC OC OC BF OC OC Q
# char set testing O X L T O C M S BB BB QQ SS AA PP O
# char set testing T C D   L T P T KK RR TT LL NN RR T
#

#
#
PREFIX="/usr"
DEFINES=""
INCLUDES=""
LINUX_RELEASE=""
MODULES_DIRECTORY=""
DRIVER_SOURCE=""

#
#
while [ ! -z "$*" ] ; do
    case "$1" in
        --pref*)
            PREFIX=`echo "$1" | awk -F'=' '{print $2}'`
            if [ -z "$PREFIX" ] ; then PREFIX="/usr" ; fi
            shift
            ;;
        --debug)
            DEFINES="$DEFINES -DCMSFS_DEBUG"
            shift
            ;;
        --*)
            echo "$Z: unrecognized option '$1'" 1>&2
            exit 24
            ;;
        -*)
            echo "$Z: unrecognized option '$1'" 1>&2
            exit 24
            ;;
        *)
            echo "$Z: unrecognized parameter '$1'" 1>&2
            exit 24
            ;;
        esac
    done

#
# is this an ASCII system?
NL=`echo "" | od -t x1 | head -1 | awk '{print $2}'`
case "$NL" in
    0a|0A|00a|a)
        DEFINES="$DEFINES -DCMSFS_HOST_ASCII"
        ;;
    15)
        DEFINES="$DEFINES -DCMSFS_HOST_EBCDIC"
        ;;
    *)
        echo "$Z: what kind of newline is '$NL'?" 1>&2
        exit 32
        ;;
    esac

#
# for the driver, use the right VFS interface shim:
case `uname -r` in

    2.2*|2.3*)
        LINUX_RELEASE="2.2"
#       ln -s cmsfs22x.c cmsfsvfs.c
        INCLUDES="-I/usr/include/linux"
        DRIVER_SOURCE="cmsfs22x.c"
        MODULES_DIRECTORY="/lib/modules/`uname -r`/fs"
        ;;
    2.4*|2.5*)
        LINUX_RELEASE="2.4"
#       ln -s cmsfs24x.c cmsfsvfs.c
        INCLUDES="-I/lib/modules/`uname -r`/build/include"
        DRIVER_SOURCE="cmsfs24x.c"
        MODULES_DIRECTORY="/lib/modules/`uname -r`/kernel/fs"
#       MODULES_DIRECTORY="/lib/modules/`uname -r`/kernel/fs/cmsfs"
        ;;
    2.6*)
        LINUX_RELEASE="2.6"
#       ln -s cmsfs24x.c cmsfsvfs.c
#        INCLUDES="-I/lib/modules/`uname -r`/build/include"
#        DRIVER_SOURCE="cmsfs24x.c"
#        MODULES_DIRECTORY="/lib/modules/`uname -r`/kernel/fs"
#       MODULES_DIRECTORY="/lib/modules/`uname -r`/kernel/fs/cmsfs"
	echo "$Z: The DRIVER does not work with Linux 2.6." 1>&2
	echo "$Z: (The utility should give you no trouble.)" 1>&2
        ;;
    3.*)
        LINUX_RELEASE="3.x"
#       ln -s cmsfs24x.c cmsfsvfs.c
#        INCLUDES="-I/lib/modules/`uname -r`/build/include"
#        DRIVER_SOURCE="cmsfs24x.c"
#        MODULES_DIRECTORY="/lib/modules/`uname -r`/kernel/fs"
#       MODULES_DIRECTORY="/lib/modules/`uname -r`/kernel/fs/cmsfs"
	echo "$Z: The DRIVER does not work with Linux 3.x." 1>&2
	echo "$Z: (The utility should give you no trouble.)" 1>&2
        ;;
    *)
        if [ `uname -s` = "Linux" ] ; then
            echo "$Z: this release of Linux is not supported!" 1>&2
            exit 28
            fi
        ;;
    esac

#
# new trick: tainted system by way of proprietary modules
if [ `uname -r` = "2.4.19" ] ; then
        DEFINES="$DEFINES -DCMSFS_LICENSED" ; fi

#
# platform-specific substutions ...
echo "s#%PREFIX%#$PREFIX#g"
echo "s#%DEFINES%#$DEFINES#g"
echo "s#%INCLUDES%#$INCLUDES#g"
echo "s#%LINUX_RELEASE%#$LINUX_RELEASE#g"
echo "s#%DRIVER_SOURCE%#$DRIVER_SOURCE#g"
echo "s#%MODULES_DIRECTORY%#$MODULES_DIRECTORY#g"
# insert required TABs into the distribution makefile ...
echo "" | awk '{printf "s#^        #\t#\n"}'

exit




