#!/bin/sh 
# 
# 
#	  Date: 2000-Sep-11 (Mon) 
#		based on a note from Jon Nolting <nolting@AMDAHL.COM>
#		outlining the syntax of the UTS command by this name 
# 
#readcms      -vmid id -addr xxx -pw pass  -m mode
#           -f fname.ftype  -o ofile -bl -bi
# 
# 
 
VMID='' ; ADDR='' ; PASS='' ; MODE='' ; NAME='' ; SAVE='' 
while [ "$1" != "" ] ; do 
case "$1" in 
 
	-vmid) 
		shift 
		VMID="$1" 
		shift 
		;; 
 
	-addr) 
		shift 
		ADDR="$1" 
		shift 
		;; 
 
	-pw) 
		shift 
		PASS="$1" 
		shift 
		;; 
		# See getpass(3C). 
 
	-m) 
		shift 
		MODE="$1" 
		shift 
		;; 
 
	-f) 
		shift 
		NAME="$1" 
		shift 
		;; 
 
	-o) 
		shift 
		SAVE="$1" 
		shift 
		;; 
 
	-bl) 
		# Causes the file specified by -f to  have  a
		# newline  character added to the end of each record. 
		# This is the default for "ASCII" (plain text) files. 
		TRAN="-a" 
		shift 
		;; 
 
	-bi) 
		TRAN="-i" 
		shift 
		;; 
 
	-*) 
		echo "$0: unrecognized option '$1'." 
		exit 24 
		;; 
 
	*) 
		break 
		;; 
 
	esac 
 
	done 
 
# 
# what does CP say about this address? 
DASD=`hcp q v "$ADDR" | awk '{print $1}'` 
if [ "$DASD" != "DASD" ] ; then 
	echo "$0: '$ADDR' is not a DASD volume." 
	exit -1 
	fi 
 
# 
# parse CP QUERY output and /proc/dasd/devices: 
ADDR=`hcp q v "$ADDR" | awk '{print $2}'` 
KDEV=`grep "^$ADDR(" /proc/dasd/devices \
	| awk -F'(' '{print $3}' | awk -F')' '{print $1}'` 
 
# 
# if we don't know, report and exit: 
if [ -z "$KDEV" ] ; then 
	echo "$0: '$ADDR' is not available to the kernel."
	exit -1 
	fi 
KDEVMAJ=`echo "$KDEV" | awk -F: '{print $1}'` 
KDEVMIN=`echo "$KDEV" | awk -F: '{print $2}'` 
KDEVTMP=/tmp/$$.dev 
mknod "$KDEVTMP" b "$KDEVMAJ" "$KDEVMIN" 
 
#echo "VMID=$VMID" 
#echo "ADDR=$ADDR" 
#echo "PASS=$PASS" 
#echo "MODE=$MODE" 
#echo "NAME=$NAME" 
#echo "SAVE=$SAVE" 
 
#echo "KDEV=$KDEV" 
#echo "KDEVMAJ=$KDEVMAJ" 
#echo "KDEVMIN=$KDEVMIN" 
#echo "KDEVTMP=$KDEVTMP" 
 
bin/cmsfscat -d "$KDEVTMP" -a "$NAME" 
rm "$KDEVTMP" 
 
 
 
 
 
 
 
