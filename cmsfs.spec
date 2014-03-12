#
#
#         Name: CMSFS SPEC (RPM package specs file)
#               RPM package specs file for the CMS FS driver and util
#         Date: 2001-Nov-30 (Fri)
#               2010-Feb-05 (Fri)
#
#

Name: cmsfs
Version: 1.1
Release: 10
Summary: CMS Filesystem driver and utility
Group: Utilities/System
License: GPL

%description
CMS Filesystem package
This RPM only contains the user mode (non-kernel) utility.

%prefix /usr
%files
%defattr(-,root,root)
/usr/bin/cmsfsvol
/usr/bin/cmsfslst
/usr/bin/cmsfscat
/usr/bin/cmsfsck
/usr/bin/cmsfscp
/usr/lib/libcmsfs.a





