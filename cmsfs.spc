#
#
#         Name: CMSFS SPEC (RPM package specs file)
#               RPM package specs file for the CMS FS driver and util
#         Date: 2001-Nov-30 (Fri)
#
#
 
Name: cmsfs
Version: 1.1
Release: 8c
Summary: CMS Filesystem driver and utility
Group: Utilities/System
License: GPL
 
#         Note! The "v" and "r" of the VRM (version) should match
#               between the header and this RPM "spec" file.
#               The "m" (VRM) and the "release" (RPM) might not.
 
#Requires: libc
 
#Provides: cmsfs
#%provides
 
%description
CMS Filesystem package
This RPM only contains the user mode (non-kernel) utility.
 
#
#       -bp    Executes the "%prep"  stage  from  the  spec  file.
#              Normally  this  involves  unpacking the sources and
#              applying any patches.
#
##%prep
##echo "running PREP stage"
# runs from /usr/src/packages/BUILD
# (runs from $RPM_BUILD_DIR)
#echo PACKAGE_VERSION=%PACKAGE_VERSION # set by "Version:" above
#echo PACKAGE_RELEASE=%PACKAGE_RELEASE # set by "Release:" above
#RPM_ARCH=i386
#RPM_OPT_FLAGS='-O2 -m486 -fno-strength-reduce'
#RPM_OS=linux
#RPM_PACKAGE_NAME=CMSFS
#RPM_PACKAGE_RELEASE=1
#RPM_PACKAGE_VERSION=1.1
#RPM_SOURCE_DIR=/usr/src/packages/SOURCES
#RPM_BUILD_DIR=/usr/src/packages/BUILD
#RPM_DOC_DIR=/usr/share/doc/packages
 
#
#       -bl    Do a "list check".  The "%files" section  from  the
#              spec file is macro expanded, and checks are made to
#              verify that each file exists.
#
%prefix /usr
%files
%defattr(-,root,root)
/usr/bin/cmsfsvol
/usr/bin/cmsfslst
/usr/bin/cmsfscat
/usr/bin/cmsfsck
/usr/bin/cmsfscp
##/sbin/fsck.cms
/usr/lib/libcmsfs.a
##/lib/modules/$RELEASE/fs/cmsfs.o
##/lib/modules/2.2.16/fs/cmsfs.o
##/lib/modules/2.4.7/kernel/fs/cmsfs/cmsfs.o
##/lib/modules/2.4.9/kernel/fs/cmsfs/cmsfs.o
##/lib/modules/2.4.19/kernel/fs/cmsfs.o, or 
##/lib/modules/2.4.19/kernel/fs/cms/cmsfs.o, or 
##/lib/modules/2.4.19/kernel/fs/cmsfs/cmsfs.o
#
#
 
#
#       -bc    Do the "%build" stage from  the  spec  file  (after
#              doing the prep stage).  This generally involves the
#              equivalent of a "make".
#
##%build
##echo "running 'build' stage"
 
#
#       -bi    Do the "%install" stage from the spec  file  (after
#              doing  the  prep and build stages).  This generally
#              involves the equivalent of a "make install".
#
##%install
##echo "running INSTALL stage"
 
#
# post-installation steps:
##%post
##/opt/BMCS/cmsfs/cmsfsins.sh
# (This section isn't as noisy as the others.)
 
 
# rpm -[b|t]O [build-options] <package_spec>+
 
#       -bb    Build a  binary  package  (after  doing  the  prep,
#              build, and install stages).
 
#       -bs    Build  just  the  source  package  (after doing the
#              prep, build, and install stages).
 
#       -ba    Build binary and source packages (after  doing  the
#              prep, build, and install stages).
 
 
#
# This is how Rick wishes the syntax would be ...
# rpmfilename:    %{NAME}-%{VERSION}-%{OS}-%{ARCH}.rpm
#
 
 
