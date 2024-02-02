#!/bin/sh
#
#
#               CMSFSRPM SH (shell script)
#
#
#
#
#

# run from the resident directory
cd `dirname "$0"`
D=`pwd`

# establish certain variables
APPLID=cmsfs
APPVRM=`grep '^#define' cmsfs.h | grep CMSFS_VERSION | awk '{print $3}' | sed 's#"##g'`
BUILDD=$D/$$

if [ ! -s .rpmseq ] ; then echo "0" > .rpmseq ; fi
RPMSEQ=`cat .rpmseq`

UNAMEM=`uname -m`
#UNAMEM=`uname -m | sed 's#^i.86$#i386#' | sed 's#^armv.l$#arm#'`

# create the "sed file"
rm -f cmsfsrpm.sed
echo "s#%APPLID%#$APPLID#g" >> cmsfsrpm.sed
echo "s#%APPVRM%#$APPVRM#g" >> cmsfsrpm.sed
echo "s#%BUILDD%#$BUILDD#g" >> cmsfsrpm.sed
echo "s#%RPMSEQ%#$RPMSEQ#g" >> cmsfsrpm.sed
echo "s#%UNAMEM%#$UNAMEM#g" >> cmsfsrpm.sed

# process the skeletal spec file into a usable spec file
sed -f cmsfsrpm.sed < cmsfs.spec > cmsfsrpm.spec
RC=$? ; if [ $RC -ne 0 ] ; then exit $RC ; fi
rm cmsfsrpm.sed

#
# clean up from any prior run
make clean 1> /dev/null 2> /dev/null
rm -rf $BUILDD
#find . -print | grep ';' | xargs -r rm

#
# configure for test
./configure --prefix=$BUILDD
RC=$? ; if [ $RC -ne 0 ] ; then exit $RC ; fi

#
# 'just make'
make
RC=$? ; if [ $RC -ne 0 ] ; then exit $RC ; fi

#
# now try an install
make install
RC=$? ; if [ $RC -ne 0 ] ; then exit $RC ; fi

#
# make it "properly rooted"
mkdir $BUILDD/usr
mv $BUILDD/bin $BUILDD/lib $BUILDD/man $BUILDD/usr/.
RC=$? ; if [ $RC -ne 0 ] ; then exit $RC ; fi

#
# build the RPM file (and keep a log of the process)
rm -f cmsfsrpm.log
rpmbuild -bb --nodeps cmsfsrpm.spec | tee cmsfsrpm.log
RC=$? ; if [ $RC -ne 0 ] ; then exit $RC ; fi
rm cmsfsrpm.spec

#
# recover the RPM file
cp -p $HOME/rpmbuild/RPMS/$UNAMEM/$APPLID-$APPVRM-$RPMSEQ.$UNAMEM.rpm .

# increment the sequence number for the next build
expr $RPMSEQ + 1 > .rpmseq

exit


