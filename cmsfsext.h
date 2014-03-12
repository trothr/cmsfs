/*
 *
 *        Name: cmsfsext.h
 *              CMS FS file EXTensions table
 *      Author: Rick Troth, BMC Software, Inc., Houston, Texas, USA
 *        Date: 2000-Sep-10 (Sun)
 *
 *              Well-known ASCII (plain text) file extensions
 *              taken from linux/fs/fat/misc.c which was
 *              written 1992, 1993 by Werner Almesberger.
 *              The table structure is completely different.
 *              Expanded to 8-byte filetypes and then
 *              well-known IMAGE (binary) file extensions added
 *              by Rick Troth for the CMS FS util and driver.
 *
 *              With the driver, the table is ultimately
 *              replaceable through use of an ioctl().
 *              For the time being, replacement is not implemented.
 *
 *              See cmsfs.h for constants,
 *              typically one of CMSFSTXT, CMSFSBIN, or CMSFSRAW.
 *              Use CMSFSANY elsewhere to indicate automatic selection.
 *              Do NOT use CMSFSANY in the table below.
 *              CMSFSANY in the table below indicates end-of-list.
 *
 *              Wildcards here are for future plans;
 *              they are not presently supported in driver or util.
 *
 */
 
/* ------------------------------------------------------------ CMSFSEXT
 */
static struct CMSFSEXT cmsfsext[] = {
/*  Well-known ASCII (plain text) file extensions  */
/* text files */
        {       "TXT     ", CMSFSTXT},
        {       "HTM     ", CMSFSTXT},
        {       "HTML    ", CMSFSTXT},
        {       "GML     ", CMSFSTXT},
        {       "SGML    ", CMSFSTXT},
        {       "LOG     ", CMSFSTXT},
        {       "ME      ", CMSFSTXT},  /*  as in "READ ME" or "read.me"  */
        {       "1ST     ", CMSFSTXT},  /*  "README 1ST" or "readme.1st"  */
        {       "FIRST   ", CMSFSTXT},  /*  "README FIRST" or "readme.first"  */
        {       "README  ", CMSFSTXT},
/* programming languages */
        {       "C       ", CMSFSTXT},
        {       "H       ", CMSFSTXT},
        {       "CPP     ", CMSFSTXT},
        {       "LIS     ", CMSFSTXT},
        {       "LIST    ", CMSFSTXT},
        {       "LISTING ", CMSFSTXT},
        {       "LST     ", CMSFSTXT},
        {       "PS      ", CMSFSTXT},
        {       "FILELIST", CMSFSTXT},
        {       "PAS     ", CMSFSTXT},
        {       "PASCAL  ", CMSFSTXT},
        {       "PLI     ", CMSFSTXT},
        {       "PLIOPT  ", CMSFSTXT},
        {       "FOR     ", CMSFSTXT},
        {       "FORT    ", CMSFSTXT},
        {       "FORTRAN ", CMSFSTXT},
        {       "MACRO   ", CMSFSTXT},
        {       "ASSEMBLE", CMSFSTXT},
        {       "ASM     ", CMSFSTXT},
        {       "COPY    ", CMSFSTXT},
/* programming languages and tools */
        {       "F       ", CMSFSTXT},
        {       "MK      ", CMSFSTXT},
        {       "MAK     ", CMSFSTXT},
        {       "MAKE    ", CMSFSTXT},
        {       "MAKEFILE", CMSFSTXT},
        {       "CNTRL   ", CMSFSTXT},
        {       "AUX*    ", CMSFSTXT},
        {       "MAP     ", CMSFSTXT},
        {       "INC     ", CMSFSTXT},
        {       "INCLUDE ", CMSFSTXT},
        {       "BAS     ", CMSFSTXT},
        {       "BASIC   ", CMSFSTXT},
        {       "HELP*   ", CMSFSTXT},
        {       "EPS     ", CMSFSTXT},
/* various scripting languages */
        {       "BAT     ", CMSFSTXT},
        {       "CMD     ", CMSFSTXT},
        {       "SH      ", CMSFSTXT | CMSFSEXE},
        {       "EXEC    ", CMSFSTXT},
        {       "REXX    ", CMSFSTXT},
        {       "XEDIT   ", CMSFSTXT},
        {       "CGI     ", CMSFSTXT},
        {       "TCL     ", CMSFSTXT | CMSFSEXE},
        {       "TK      ", CMSFSTXT | CMSFSEXE},
        {       "PL      ", CMSFSTXT | CMSFSEXE},
        {       "PHP     ", CMSFSTXT | CMSFSEXE},
/* config files */
        {       "INI     ", CMSFSTXT},
        {       "CONFIG  ", CMSFSTXT},
        {       "CFG     ", CMSFSTXT},
        {       "TCPXLATE", CMSFSTXT},
        {       "RTABLE  ", CMSFSTXT},
        {       "PSEG    ", CMSFSTXT},
        {       "DIRECT  ", CMSFSTXT},  /*  equiv to VMware "vmx" or "cfg"  */
        {       "VMX     ", CMSFSTXT},  /*  yes, VMX = VMware, not VM/ESA  */
        {       "ENV     ", CMSFSTXT},
/* graphics */
        {       "PBM     ", CMSFSTXT},
        {       "PGM     ", CMSFSTXT},
        {       "DXF     ", CMSFSTXT},
/* TeX */
        {       "TEX     ", CMSFSTXT},
        {       "XML     ", CMSFSTXT},
/* databases */
        {       "NAMES   ", CMSFSTXT},
        {       "NODES   ", CMSFSTXT},
/* more than I care to document ... */
        {       "NOTE    ", CMSFSTXT},
        {       "MAIL    ", CMSFSTXT},
        {       "VCF     ", CMSFSTXT},  /*  VCard  */
        {       "VCARD   ", CMSFSTXT},
        {       "SYNONYM ", CMSFSTXT},
/* documentation (man pages) */
        {       "1       ", CMSFSTXT},
        {       "2       ", CMSFSTXT},
        {       "3       ", CMSFSTXT},
        {       "4       ", CMSFSTXT},
        {       "5       ", CMSFSTXT},
        {       "6       ", CMSFSTXT},
        {       "7       ", CMSFSTXT},
        {       "8       ", CMSFSTXT},
        {       "9       ", CMSFSTXT},
        {       "N       ", CMSFSTXT},
/*  Well-known IMAGE (binary) file extensions  */
        {       "BIN     ", CMSFSBIN},
        {       "EXE     ", CMSFSBIN},
        {       "DLL     ", CMSFSBIN},
        {       "O       ", CMSFSBIN},
        {       "OBJ     ", CMSFSBIN},
        {       "A       ", CMSFSBIN},
        {       "BOO     ", CMSFSBIN},
        {       "BOOK    ", CMSFSBIN},
        {       "TXTLIB  ", CMSFSBIN},
        {       "TXT*    ", CMSFSBIN},  /*  hoping it's F 80  */
        {       "TCPXLBIN", CMSFSBIN},  /*  hoping it's F 256  */
        {       "GIF     ", CMSFSBIN},
        {       "JPG     ", CMSFSBIN},
        {       "JPEG    ", CMSFSBIN},
        {       "MPG     ", CMSFSBIN},
        {       "MPEG    ", CMSFSBIN},
        {       "TIF     ", CMSFSBIN},
        {       "TIFF    ", CMSFSBIN},
        {       "MOV     ", CMSFSBIN},
        {       "MOVIE   ", CMSFSBIN},
        {       "AVI     ", CMSFSBIN},
        {       "MP3     ", CMSFSBIN},
        {       "WAV     ", CMSFSBIN},
        {       "VMARC   ", CMSFSBIN},  /*  hoping it's F 80 (!)  */
        {       "ZIP     ", CMSFSBIN},
        {       "Z       ", CMSFSBIN},
        {       "GZ      ", CMSFSBIN},
        {       "TAR     ", CMSFSBIN},
        {       "TAZ     ", CMSFSBIN},
        {       "TGZ     ", CMSFSBIN},
        {       "PGP     ", CMSFSBIN},
        {       "RPM     ", CMSFSBIN},
        {       "DEB     ", CMSFSBIN},
/*  filesystems within filesystems  */
        {       "ISO     ", CMSFSBIN},
        {       "CD-ROM  ", CMSFSBIN},
        {       "CDROM   ", CMSFSBIN},
        {       "EXT     ", CMSFSBIN},
        {       "EXT2    ", CMSFSBIN},
        {       "EXT2FS  ", CMSFSBIN},
        {       "RAMDISK ", CMSFSBIN},
        {       "FAT     ", CMSFSBIN},
        {       "VFAT    ", CMSFSBIN},
/*  Well-known extensions for record-oriented file types  */
        {       "LIST3820", CMSFSRAW},
        {       "MODULE  ", CMSFSRAW},
        {       "LOADLIB ", CMSFSRAW},
/*  other stuff  */
        {       "URL     ", CMSFSTXT},
        {       "LNK     ", CMSFSBIN},
/* the terminator */
        {       "ALLOCMAP", CMSFSBIN},
        {       "DIRECTOR", CMSFSBIN},  /*  better be F 64!  */
 
        {       "        ", CMSFSANY}   /*  CMSFSANY marks end  */
};
 
 
