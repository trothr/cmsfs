#!/bin/sh
#
#
#         Name: _REFRESH SH
#       Author: Rick Troth, BMC Software, Inc., Houston, Texas, USA
#         Date: 2000-Nov-09 (Thu) and prior and later
#
#
 
SOURCE="$1"
TMPDIR=/tmp/cmsfstmp
 
#
# special case for OpenVM:
if [ `uname -s` = "VM/ESA" -o `uname -s` = "z/VM" ] ; then
        echo "Special processing for VM (CMS) ..."
 
        LA=`basename "$SOURCE"`
        FM="E"
 
        echo " ACCESS $LA $FM " ; RC="$?"
        cms " ACCESS $LA $FM " ; RC="$?"
        if [ "$RC" != 0 ] ; then exit $RC ; fi
 
        #
        # listing the files, refresh each found on CMS media:
        for F in `cat //cmsfs.filelist.$FM | grep '^ ' \
                        | awk '{print $1 "." $2}' | tr A-Z a-z` ; do
                FN=`echo "$F" | awk -F. '{print $1}'`
                FT=`echo "$F" | awk -F. '{print $2}'`
                echo " openvm putbfs $FN $FT $FM $F ( olddate replace "
                cms " openvm putbfs $FN $FT $FM $F ( olddate replace "
                RC="$?"
                if [ "$RC" != 0 ] ; then exit $RC ; fi
                done
 
        exit 0
        fi
 
#
# sanity check: is the source a device?
if [ ! -b "$SOURCE" ] ; then
        echo "$0: '$SOURCE' is not a block device!"
        exit -1
        fi
 
#
# sanity check: is the source readable?
if [ ! -r "$SOURCE" ] ; then
        echo "$0: '$SOURCE' is not readable!"
        exit -1
        fi
 
#
# creat a temporary staging directory:
mkdir "$TMPDIR"
 
#
# listing the files, refresh each found on CMS media:
for F in `bin/cmsfscat -d "$SOURCE" -a cmsfs.filelist | grep '^ ' \
                | awk '{print $1 "." $2}' | tr A-Z a-z` ; do
#       bin/cmsfscp -d "$SOURCE" -a "$F" "$TMPDIR/$F"
        bin/cmsfscat -d "$SOURCE" -a "$F" > "$TMPDIR/$F"
        if [ "$?" != 0 ] ; then continue ; fi
        new -d "$TMPDIR/$F" . 1>/dev/null 2>/dev/null
        if [ "$?" != 0 ] ; then cp -p "$TMPDIR/$F" . ; fi
        done
 
#
# NOTE: we would copy 'cp' instead of concat 'cat'
# except that CMS FS stores file times in localtime
# and both driver and util convert that to GMT.  Bad!
#
 
#
# clean-up after ourselves:
rm -r "$TMPDIR"
 
#
# make shell scripts executable:
chmod +x *.sh
 
exit 0
 
#
# copy certain files to more popular names:
cat cmsfs.filelist | grep -v '^*' \
        | awk '{print $4 " " $1 "." $2}' | grep -v '^ ' \
        | awk '{print "ln -s " tolower($2) " " $1}' | sh 2>/dev/null
#       | awk '{print "new -k " tolower($2) " " $1}' | sh
 
exit 0
 
