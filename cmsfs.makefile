#
#
#
#         Name: makefile (make rules file)
#               'make' rules for the CMS FS driver and utility
#               Source maintained as CMSFS MAKEFILE
#               (aka "cmsfs.makefile") and processed by 'configure'.
#
#
#

#
# real-live CMS volume where these things are maintained
#XEDITSRC        =       /dev/dsk/01B5
#XEDITSRC        =       /dev/dasdf
XEDITSRC        =       dsk/cms1fs.dsk

#
# a mount-point to test the driver
#CMSMOUNT        =       /CMS/CMS1FS
CMSMOUNT        =       /tmp/cms1fs

#
# platform-specific definitions (substituted by 'configure')
DEFINES         =       %DEFINES%
INCLUDES        =       %INCLUDES%
PREFIX          =       %PREFIX%
LINUX_RELEASE   =       %LINUX_RELEASE%
DRIVER_SOURCE   =	%DRIVER_SOURCE%
MODULES_DIRECTORY  =    %MODULES_DIRECTORY%

#
# list of target objects for the utility
OBJECTS         =       cmsfscat cmsfslst cmsfsvol cmsfsck cmsfscp

#
# when the user does a "just make" easy build
_default:       $(OBJECTS)

#
# all the usual products
all:            $(OBJECTS) libcmsfs.a

#
############################################################### MAKEFILE
make:           makefile

makefile:       cmsfs.makefile cmsfscfg.sed makefile.in
		@rm -f makefile-OLD makefile.old
		@if [ -f makefile ] ; then mv makefile makefile-OLD ; fi
		sed -f cmsfscfg.sed < cmsfs.makefile > makefile
		@echo "$(MAKE): the makefile has been updated"
		@echo " "

makefile.in:    cmsfs.makefile
		@rm -f makefile.in
		cp -p cmsfs.makefile makefile.in
		@touch makefile.in

cmsfscfg.sed:   cmsfssed.sh
		rm -f cmsfscfg.sed
		@chmod +x cmsfssed.sh
		sh -c ' exec ./cmsfssed.sh ' > cmsfscfg.sed

#cmsfsmak.sed:	cmsfssed.sh
#		rm -f cmsfsmak.sed
#		@chmod +x cmsfssed.sh
#		sh -c ' exec ./cmsfssed.sh ' > cmsfsmak.sed

#
################################################################# CONFIG
config:         configure
                @chmod +x configure
                sh -c ' ./configure '
#               @echo $(MAKE) makefile
#		@echo @rm -f cmsfsvfs.c
#		@echo $(MAKE) cmsfsvfs.c

configure:      cmsfscfg.sh
		@rm -f configure-OLD
             @if [ -f configure ] ; then mv configure configure-OLD ; fi
                cp cmsfscfg.sh configure
                chmod +x configure

cmsfsvfs.c:	$(DRIVER_SOURCE)
		@rm -f cmsfsvfs.c
		@if [ ! -z "$(DRIVER_SOURCE)" ] ; then \
			echo "    ln -s $(DRIVER_SOURCE) cmsfsvfs.c" ; \
			ln -s $(DRIVER_SOURCE) cmsfsvfs.c ; fi

#
################################################################### DOCS
docs doc :	README LICENSE INSTALL CREDITS cmsfsext.html

README:         cmsfs.txt
                @if [ -f README ] ; then mv README README-OLD ; fi
                cp -p cmsfs.txt README

LICENSE:        cmsfsgpl.txt
                @if [ -f LICENSE ] ; then mv LICENSE LICENSE-OLD ; fi
                cp -p cmsfsgpl.txt LICENSE

INSTALL:        cmsfsmak.txt
                @if [ -f INSTALL ] ; then mv INSTALL INSTALL-OLD ; fi
                cp -p cmsfsmak.txt INSTALL

CREDITS:        cmsfswho.txt
                @if [ -f CREDITS ] ; then mv CREDITS CREDITS-OLD ; fi
                cp -p cmsfswho.txt CREDITS

cmsfsext.html:	makefile cmsfsext.h cmsfsext.htmlhead cmsfsext.htmltail 
		@rm -f cmsfsext.html 
		cat cmsfsext.htmlhead > cmsfsext.tmp 
		grep CMSFS cmsfsext.h | grep ',' | grep '"' \
			| awk -F'{' '{print $$2}' \
			| awk -F'}' '{print $$1}' \
			| grep -v CMSFSANY | grep -v '*' \
			| sed 's/CMSFSTXT/plain text/' \
			| sed 's/CMSFSEXE/executable/' \
			| sed 's/CMSFSBIN/binary/' \
		      | sed 's/CMSFSRAW/binary (with record lengths)/' \
			| sed 's#"#<td><tt>#' | sed 's#"#</tt><td>#' \
			| sed 's/,/ /g' | sed 's/|/,/g' \
		      | awk '{print "<tr>" $$0 "</tr>"}' >> cmsfsext.tmp 
		cat cmsfsext.htmltail >> cmsfsext.tmp 
		mv cmsfsext.tmp cmsfsext.html 
 
########################################################################

#
# 'cmsfscat', write (concatenate) CMS files to stdout
cmsfscat:       cmsfscat.o libcmsfs.a
                $(CC) $(DEFINES) -o cmsfscat cmsfscat.o -L. -lcmsfs
#               strip cmsfscat

cmsfscat.o:     cmsfscat.c cmsfs.h aecs.h
                $(CC) $(DEFINES) -c cmsfscat.c

#
# 'cmsfslst', list CMS files in LISTFILE format
cmsfslst:       cmsfslst.o libcmsfs.a
                $(CC) $(DEFINES) -o cmsfslst cmsfslst.o -L. -lcmsfs
#               strip cmsfslst

cmsfslst.o:     cmsfslst.c cmsfs.h aecs.h
                $(CC) $(DEFINES) -c cmsfslst.c

#
# 'cmsfsvol', display CMS volume info in QUERY DISK format
cmsfsvol:       cmsfsvol.o libcmsfs.a
                $(CC) $(DEFINES) -o cmsfsvol cmsfsvol.o -L. -lcmsfs
#               strip cmsfsvol

cmsfsvol.o:     cmsfsvol.c cmsfs.h aecs.h
                $(CC) $(DEFINES) -c cmsfsvol.c

#
# 'cmsfsck', filesystem check for CMS volumes
cmsfsck:        cmsfsck.o libcmsfs.a
                $(CC) $(DEFINES) -o cmsfsck cmsfsck.o -L. -lcmsfs
#               strip cmsfsck

cmsfsck.o:      cmsfsck.c cmsfs.h aecs.h
                $(CC) $(DEFINES) -c cmsfsck.c

#
# 'cmsfscp', copy files from a CMS volume
cmsfscp:        cmsfscp.o libcmsfs.a
                $(CC) $(DEFINES) -o cmsfscp cmsfscp.o -L. -lcmsfs
#               strip cmsfscp

cmsfscp.o:      cmsfscp.c cmsfs.h aecs.h
                $(CC) $(DEFINES) -c cmsfscp.c

#
# 'cmsfsls', list CMS files in Unix 'ls' format
cmsfsls:        cmsfsls.o libcmsfs.a
                $(CC) $(DEFINES) -o cmsfsls cmsfsls.o -L. -lcmsfs
#               strip cmsfsls

cmsfsls.o:      cmsfsls.c cmsfs.h aecs.h
                $(CC) $(DEFINES) -c cmsfsls.c

#
#
libcmsfs.a:     cmsfsusa.c cmsfsany.c cmsfs.h aecs.o aecs.h
                $(CC) $(DEFINES) \
                        -c cmsfsusa.c -o cmsfsusa.o
                ar q libcmsfs.a cmsfsusa.o aecs.o

#
#
aecs.o:         aecs.c aecs.h
                $(CC) $(DEFINES) -c aecs.c

#
# driver module
cmsfs.o:        makefile cmsfsvfs.c cmsfsany.c cmsfs.h aecs.c aecs.h
                $(CC) $(DEFINES) -O2 -DMODULE $(INCLUDES) \
                        -c cmsfsvfs.c -o cmsfs.o

########################################################################

#
#
utility util:   $(OBJECTS)

#
#
driver:         /usr/src/linux/fs/cmsfs

#
#
module modules :  cmsfs.o

# 
# 
test:		cmsfsck cmsfscat cmsfslst cmsfscp cmsfsvol 
		./cmsfsck dsk/cms1fs.dsk 
	       ./cmsfscat -d dsk/cms1fs.dsk -a cmsfs.makefile >/dev/null 
		./cmsfslst -d dsk/cms1fs.dsk                  >/dev/null 
	     ./cmsfscp -d dsk/cms1fs.dsk -p -a cmsfs.makefile cms1fs.tmp 
		@rm cms1fs.tmp 
		./cmsfsvol dsk/cms1fs.dsk 
		@echo "$(MAKE): utility passed all tests" 


########################################################################
/usr/src/linux/fs/cmsfs:  cmsfsvfs.c cmsfsany.c cmsfsvfs.filelist \
			cmsfs.h aecs.c aecs.h
                mkdir /usr/src/linux/fs/cmsfs
                @cat cmsfsvfs.filelist | grep -v '^*' \
                    | awk '{print tolower($$1) "." tolower($$2) " " \
                        $$4 " " tolower($$1) "." tolower($$2)}' \
                    | awk '{print "cp -p " $$1 \
                        " /usr/src/linux/fs/cmsfs/" $$2}' | sh -x
########################################################################

#
#
install:        $(OBJECTS) libcmsfs.a
		mkdir -p $(PREFIX)/bin
                for F in $(OBJECTS) ; do \
                        rm -f $(PREFIX)/bin/$$F ; \
                        cp -p $$F $(PREFIX)/bin/. ; done
		mkdir -p $(PREFIX)/lib
                rm -f $(PREFIX)/lib/libcmsfs.a
                cp -p libcmsfs.a $(PREFIX)/lib/.
###             $(MAKE) /sbin/fsck.cms
#               touch install
		mkdir -p $(PREFIX)/man/man8
		cp -p *.8 $(PREFIX)/man/man8/.

#
#
module_install modules_install :  cmsfs.o cmsfsck
		@test ! -z "$(MODULES_DIRECTORY)"
#/sbin/fsck.cms:  $(PREFIX)/bin/cmsfsck
		rm -f $(MODULES_DIRECTORY)/cmsfs.o
		cp -p cmsfs.o $(MODULES_DIRECTORY)/.
                rm -f /sbin/fsck.cms
                cp -p cmsfsck /sbin/fsck.cms

#
#
rpm:            install
                test "$(PREFIX)" = "/usr" 
                rm -f rpm.log
#                rpm -bb cmsfs.spec | tee rpm.log
                rpmbuild -bb cmsfs.spec | tee rpm.log
                grep '^Wrote:' rpm.log \
                        | awk '{print "mv", $$2, "."}' | sh
                rm rpm.log

########################################################################

#
# get the latest from the development minidisk
_refresh refresh:  cmsfslst cmsfscp $(XEDITSRC)
		@test -r $(XEDITSRC)
		mkdir -p cmsfstmp
		./cmsfslst -d $(XEDITSRC) | tail +4 \
			| awk '{print $$1 "." $$2}' \
			| grep -v ':' | grep -v '.-.' \
			| tr A-Z a-z > cmsfstmp/.list
		for F in `cat cmsfstmp/.list` ; do \
			./cmsfscp -d $(XEDITSRC) \
				-p -a $$F cmsfstmp/$$F ; \
			done
		rm cmsfstmp/.list
#		sh -c ' new -d cmsfstmp/* . '
		rm -r cmsfstmp

#
#
cmsfsedf.bin:   cmsfsedf-$(HOSTTYPE).bin

cmsfsedf-CMS.bin:
        @state CMSFS FILELIST *
        @pipe command LISTFILE CMSFS FILELIST * \
          | take first \
          | spec w 3 1 \
          | spec /Q DISK/ nw 1.1 nw \
          | command \
          | drop first \
          | spec 7-* \
          | spec /mdiskblk/ nw w 2 nw /1-/ nw w 10 n \
          | spec 1-* 1 /! > cmsfsedf bin a f 4096/ nw \
          | change x5A x4F \
          | runpipe

cmsfsedf-i386.bin:
	@echo "$(MAKE): you need to be on CMS to do this"
	@sh -c ' exit 24 '

#
# exercise this stuff
_testrun testrun:       $(OBJECTS) readcms.sh _testrun.sh
                @if [ ! -d bin ] ; then mkdir bin ; fi
                @if [ ! -d dsk ] ; then mkdir dsk ; fi
                -dd if=$(XEDITSRC) of=dsk/TESTDISK
                ./cmsfsvol -d dsk/TESTDISK > /dev/null
                @new -d cmsfsvol bin/.
                ./cmsfslst -d dsk/TESTDISK > /dev/null
                @new -d cmsfslst bin/.
                ./cmsfscat -d dsk/TESTDISK -a cmsfs.filelist > /dev/null
                @new -d cmsfscat bin/.
                ./cmsfsck dsk/TESTDISK
                @new -d cmsfsck bin/.
                ./cmsfscp -d dsk/TESTDISK -a cmsfs.makefile testmake
                @new -d cmsfscp bin/.
                @chmod +x readcms.sh
                @new -k readcms.sh bin/readcms
                sh -c ' ./_testrun.sh $(OBJECTS) readcms'

#
#
mount:          cmsfs.o
                @mkdir -p -m 555$(CMSMOUNT)
                insmod cmsfs.o
                mount -t cms $(XEDITSRC) $(CMSMOUNT)
                @echo " "
                df $(CMSMOUNT)
                @echo " "


########################################################################

#
# clean up your mess!
clean:
                rm -f *.o *.a $(OBJECTS) *.tmp *-OLD \
                        a.out core *.exe CEEDUMP* \
                        driver module install
                @rm -f testmake* newmake* testfile*
#		@rm -f makefile* Makefile* ERR

#
#
distclean veryclean :
		@$(MAKE) clean
#               rm -f makefile* Makefile*
		rm -f cmsfsvfs* *.sed
                @touch "temp:file"
                find . -type f -print | grep ':' | xargs rm
                @touch "temp;file"
                find . -type f -print | grep ';' | xargs rm

# 
# 
cmsagain:	makefile 
		ls -d *.* | grep -v ';' | grep -v ':' \
			| awk -F'.' '{print "cms openvm getbfs " $$0 \
			" " $$1 " " $$2 " a \\( olddate replace"}' \
			| sh -x 
 
#
# good idea for 'make help' to be useful
help:		makefile
                @echo "$(MAKE):"
                @echo "$(MAKE):  some targets for this directory are:"
                @echo "$(MAKE):    all, module, install, clean"
                @echo "$(MAKE):"
                @echo "$(MAKE):  configure with ./configure"
                @echo "$(MAKE):"



