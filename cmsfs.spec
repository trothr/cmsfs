#
#
#         Name: CMSFS SPEC (RPM package specs file)
#               RPM package specs file for the CMS FS driver and util
#               http://ftp.rpm.org/max-rpm/s1-rpm-build-creating-spec-file.html
#         Date: 2001-Nov-30 (Fri)
#               2010-Feb-05 (Fri)
#               2024-02-04 (Sun)
#
#   Build with:
#               # make substitutions on this file, then:
#               rpmbuild -bb --nodeps cmsfs.spec
#

# The name line defines what the package will actually be called.
Name: %APPLID%
Version: %APPVRM%
Release: %RPMSEQ%
Summary: CMS Filesystem driver and utility
Group: Utilities/System
License: GPL
#Copyright: Casita.Net

Source: cmsfs-%{version}.tar.gz
#Prefix: %SPEC_PREFIX%
#Provides: cmsfs

#URL: https://github.houston.softwaregrp.net/rtroth/ussw/archive/master.zip
##Buildroot: /tmp/cmsfs
## The distribution line identifies the product this package is part of.
#Distribution: Casita.Net
#Vendor: La Casita en la Esquina
#Packager: Sir Santa

#
%description
CMS Filesystem package
This package contains the user mode (non-kernel) utility.

# the source has already been fetched
%prep
true

# do a 'make install' and this is not needed
%build
#mkdir -p $HOME/rpmbuild/BUILDROOT/cmsfs-%SPEC_VERSION%-%SPEC_RELEASE%.%SPEC_UNAMEM%/%SPEC_PREFIX%
#mkdir -p $HOME/rpmbuild/BUILDROOT/cmsfs-1.1.12-1.x86_64
mkdir -p $HOME/rpmbuild/BUILDROOT/%APPLID%-%APPVRM%-%RPMSEQ%.%UNAMEM%
#rsync -a -u -x -H -O -S %SPEC_STAGING%/. $HOME/rpmbuild/BUILDROOT/cmsfs-%SPEC_VERSION%-%SPEC_RELEASE%.%SPEC_UNAMEM%/%SPEC_PREFIX%/.
#rsync -a -u -x -H -O -S `pwd`/subset/. $HOME/rpmbuild/BUILDROOT/cmsfs-1.1.12-1.x86_64/.
#rsync -a -u -x -H -O -S /home/rmt/devel/cmsfs/test/subset/. $HOME/rpmbuild/BUILDROOT/cmsfs-1.1.12-1.x86_64/.
rsync -a -u -x -H -O -S %BUILDD%/. $HOME/rpmbuild/BUILDROOT/%APPLID%-%APPVRM%-%RPMSEQ%.%UNAMEM%/.

# we should have done a 'make install' before this step
%install
#mkdir -p $HOME/rpmbuild/BUILDROOT/cmsfs-%SPEC_VERSION%-%SPEC_RELEASE%.%SPEC_UNAMEM%/%SPEC_PREFIX%
#mkdir -p $HOME/rpmbuild/BUILDROOT/cmsfs-1.1.12-1.x86_64
mkdir -p $HOME/rpmbuild/BUILDROOT/%APPLID%-%APPVRM%-%RPMSEQ%.%UNAMEM%
#rsync -a -u -x -H -O -S %SPEC_STAGING%/. $HOME/rpmbuild/BUILDROOT/cmsfs-%SPEC_VERSION%-%SPEC_RELEASE%.%SPEC_UNAMEM%/%SPEC_PREFIX%/.
#rsync -a -u -x -H -O -S `pwd`/subset/. $HOME/rpmbuild/BUILDROOT/cmsfs-1.1.12-1.x86_64/.
#rsync -a -u -x -H -O -S /home/rmt/devel/cmsfs/test/subset/. $HOME/rpmbuild/BUILDROOT/cmsfs-1.1.12-1.x86_64/.
rsync -a -u -x -H -O -S %BUILDD%/. $HOME/rpmbuild/BUILDROOT/%APPLID%-%APPVRM%-%RPMSEQ%.%UNAMEM%/.


#
#%prefix /usr
# The %files section is a list of the files that comprise the package.
# If it isn't in the file list, it won't be put into the package.
%files
#%defattr(-,root,root)
/usr/bin/cmsfsvol
/usr/bin/cmsfslst
/usr/bin/cmsfscat
/usr/bin/cmsfsck
/usr/bin/cmsfscp
/usr/lib/libcmsfs.a
#
/usr/man/man8/cmsfscat.8.gz
/usr/man/man8/cmsfsck.8.gz
/usr/man/man8/cmsfscp.8.gz
/usr/man/man8/cmsfsls.8.gz
/usr/man/man8/cmsfslst.8.gz
/usr/man/man8/cmsfsvol.8.gz
/usr/man/man8/cmsmount.8.gz
#
/etc/init.d/cmsfsrun
/etc/sysconfig/cmsfsrun
/usr/lib/systemd/system/cmsfsrun.service
#
/sbin/fsck.cms
#
#

%clean
# Here is where we clean-up after all the building and packaging.
true

%post
# Run the 'install.sh' after the content is deployed.
#sh /opt/vendor/package/sbin/install.sh
true


