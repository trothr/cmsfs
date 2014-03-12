#!/bin/sh
#
#
#         Name: _TESTRUN SH
#       Author: Rick Troth
#         Date: 2000-Sep-12 (Tue)
#
#
 
UNAMES=`uname -s`
UNAMEP=`uname -m`
PREFIX="/bmc/auto/ftp/pub/cmsfs"
 
case "$UNAMES" in
 
        AIX)
                UNAMEP=`uname -M | awk '{print $2}'`
                ;;
 
        Linux)
                UNAMEP=`uname -m`
                ;;
 
        SunOS)
                UNAMES="Solaris"
                UNAMEP=`uname -p`
                ;;
 
        *)
                echo "$0: unknown O/S '$UNAMES'"
                exit 32
                ;;
 
        esac
 
case "$UNAMEP" in
 
        PPS)
                UNAMEP="powerpc"
                ;;
 
        i686|i586|i486)
                UNAMEP=i386
                ;;
 
        esac
 
if [ ! -d "$PREFIX"/"$UNAMES-$UNAMEP" ] ; then
        mkdir "$PREFIX"/"$UNAMES-$UNAMEP"
        mkdir "$PREFIX"/"$UNAMES-$UNAMEP"/bin
        mkdir "$PREFIX"/"$UNAMES-$UNAMEP"/lib
        fi
 
for F in $* ; do
        new -k bin/$F "$PREFIX"/"$UNAMES-$UNAMEP"/bin/.
        done
 
for F in *.a *.o ; do
        new -k lib/$F "$PREFIX"/"$UNAMES-$UNAMEP"/lib/.
        done
 
 
