/*
 *
 *        Name: cmsfsext.c
 *		CMS FS file EXTensions table 
 *      Author: Rick Troth, BMC Software, Inc., Houston, Texas, USA
 *        Date: 2000-Sep-10 (Sun)
 *
 *              Well-known ASCII (plain text) file extensions
 *              taken from linux/fs/fat/misc.c which was
 *              written 1992, 1993 by Werner Almesberger.
 *              The table structure is completely different.
 *              Expanded to 8-byte filetypes and then
 *              well-known IMAGE (binary) file extensions added
 *              by Rick Troth for the CMS FS util/driver.
 *              The table is ultimately replaceable via ioctl().
 *
 *		See cmsfs.h for constants, typically one of 
 *		CMSFSTXT, CMSFSBIN, or CMSFSRAW. 
 */
 
 
/* cheezy way to get time def'ns in cmsfs.h to work */ 
#include	<time.h> 
/* cheezy way to get time ssize_t in cmsfs.h to work */ 
#include	<fcntl.h> 
 
#include	"cmsfs.h" 
 
    struct CMSFSEXT cmsfsext[] = { 
 
/*  Well-known ASCII (plain text) file extensions  */
/* text files */
  "TXT     ", CMSFSTXT,
  "ME      ", CMSFSTXT,         /*  as in "READ ME" or "read.me"  */
  "HTM     ", CMSFSTXT,
  "HTML    ", CMSFSTXT,
  "1ST     ", CMSFSTXT,
  "LOG     ", CMSFSTXT,
/* programming languages */
  "C       ", CMSFSTXT,
  "H       ", CMSFSTXT,
  "CPP     ", CMSFSTXT,
  "LIS     ", CMSFSTXT,
  "LIST    ", CMSFSTXT,
  "LISTING ", CMSFSTXT,
  "PS      ", CMSFSTXT,
  "FILELIST", CMSFSTXT,
  "PAS     ", CMSFSTXT,
  "PASCAL  ", CMSFSTXT,
  "PLI     ", CMSFSTXT,
  "PLIOPT  ", CMSFSTXT,
  "FOR     ", CMSFSTXT,
  "FORT    ", CMSFSTXT,
  "FORTRAN ", CMSFSTXT,
  "MACRO   ", CMSFSTXT,
  "ASSEMBLE", CMSFSTXT,
  "COPY    ", CMSFSTXT,
/* programming languages and tools */
  "F       ", CMSFSTXT,
  "MAK     ", CMSFSTXT,
  "MAKE    ", CMSFSTXT,
  "MAKEFILE", CMSFSTXT,
  "CNTRL   ", CMSFSTXT,
  "AUX*    ", CMSFSTXT,
  "MAP     ", CMSFSTXT,
  "INC     ", CMSFSTXT,
  "BAS     ", CMSFSTXT,
  "BASIC   ", CMSFSTXT,
  "HELP*   ", CMSFSTXT,
  "EPS     ", CMSFSTXT,
/* various scripting languages */
  "BAT     ", CMSFSTXT,
  "CMD     ", CMSFSTXT,
  "SH      ", CMSFSTXT | CMSFSEXE,
  "EXEC    ", CMSFSTXT,
  "REXX    ", CMSFSTXT,
  "XEDIT   ", CMSFSTXT,
  "CGI     ", CMSFSTXT,
  "TCL     ", CMSFSTXT | CMSFSEXE,
  "TK      ", CMSFSTXT | CMSFSEXE,
/* config files */
  "INI     ", CMSFSTXT,
  "CONFIG  ", CMSFSTXT,
  "CFG     ", CMSFSTXT,
  "TCPXLATE", CMSFSTXT,
  "RTABLE  ", CMSFSTXT,
  "PSEG    ", CMSFSTXT,
  "DIRECT  ", CMSFSTXT,        /*  equiv to VMware "vmx" or "cfg" types  */
  "VMX     ", CMSFSTXT,         /*  yes, that's VMware, not VM/ESA  */
/* graphics */
  "PBM     ", CMSFSTXT,
  "PGM     ", CMSFSTXT,
  "DXF     ", CMSFSTXT,
/* TeX */
  "TEX     ", CMSFSTXT,
  "XML     ", CMSFSTXT,
/* databases */
  "NAMES   ", CMSFSTXT,
  "NODES   ", CMSFSTXT,
/* more than I care to document ... */
  "NOTE    ", CMSFSTXT,
  "MAIL    ", CMSFSTXT,
  "VCF     ", CMSFSTXT,         /*  VCard  */
  "VCARD   ", CMSFSTXT,
  "SYNONYM ", CMSFSTXT,
 
/*  Well-known IMAGE (binary) file extensions  */
  "BIN     ", CMSFSBIN,
  "EXE     ", CMSFSBIN,
  "DLL     ", CMSFSBIN,
  "BOO     ", CMSFSBIN,
  "BOOK    ", CMSFSBIN,
  "TXTLIB  ", CMSFSBIN,
  "TXT*    ", CMSFSBIN,         /*  hoping it's F 80  */
  "TCPXLBIN", CMSFSBIN,         /*  hoping it's F 256  */
  "GIF     ", CMSFSBIN,
  "JPG     ", CMSFSBIN,
  "JPEG    ", CMSFSBIN,
  "MPG     ", CMSFSBIN,
  "MPEG    ", CMSFSBIN,
  "TIF     ", CMSFSBIN,
  "TIFF    ", CMSFSBIN,
  "MOV     ", CMSFSBIN,
  "MOVIE   ", CMSFSBIN,
  "AVI     ", CMSFSBIN,
  "VMARC   ", CMSFSBIN,         /*  hoping it's F 80 (!)  */
  "ZIP     ", CMSFSBIN,
  "Z       ", CMSFSBIN,
  "GZ      ", CMSFSBIN,
  "TAR     ", CMSFSBIN,
  "TAZ     ", CMSFSBIN,
  "TGZ     ", CMSFSBIN,
  "PGP     ", CMSFSBIN,
  "RPM     ", CMSFSBIN,
 
/*  Well-known record-oriented file extensions  */
  "LIST3820", CMSFSRAW,
  "MODULE  ", CMSFSRAW,
  "LOADLIB ", CMSFSRAW,
 
/* the terminator */ 
  "ALLOCMAP", CMSFSBIN,
  "DIRECTOR", CMSFSBIN		/*  better be F 64  */ 
 
		} ; 
 
 
