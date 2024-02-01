#
#
#         Name: CMSFS SPEC (RPM package specs file)
#               RPM package specs file for the CMS FS driver and util
#         Date: 2001-Nov-30 (Fri)
#               2010-Feb-05 (Fri)
#               2024-02-04 (Sun)
#

Name: cmsfs
Version: 1.1.12
Release: 10
Summary: CMS Filesystem driver and utility
Group: Utilities/System
License: GPL
#
%description
CMS Filesystem package
This RPM only contains the user mode (non-kernel) utility.
#
%prefix /usr
%files
%defattr(-,root,root)
/usr/bin/cmsfsvol
/usr/bin/cmsfslst
/usr/bin/cmsfscat
/usr/bin/cmsfsck
/usr/bin/cmsfscp
/usr/lib/libcmsfs.a
#
/usr/man/man8/cmsfscat.8
/usr/man/man8/cmsfsck.8
/usr/man/man8/cmsfscp.8
/usr/man/man8/cmsfsls.8
/usr/man/man8/cmsfslst.8
/usr/man/man8/cmsfsvol.8
/usr/man/man8/cmsmount.8
#
/etc/init.d/cmsfsrun 
/etc/sysconfig/cmsfsrun
/usr/lib/systemd/system/cmsfsrun.service
#
#
