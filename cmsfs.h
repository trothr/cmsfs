/*
 *
 *        Name: linux/fs/cmsfs/cmsfs.h (C source header file)
 *              preprocessor definitions for the CMS filesystem package
 *        Date: 2000-Jun-14 (Wed)
 *              Copyright © 2001, 2003, distributed under GPL
 *              Rick Troth <rtroth@bmc.com>
 *              BMC Software, Inc., Houston, Texas, USA
 *              Rick Troth <rmt@casita.net>
 *
 *              Some specifications contained here were taken from
 *              http://www.vm.ibm.com/pubs/cmsdacb/FSTD.HTML (stale 2014),
 *              http://www.vm.ibm.com/pubs/cmsdacb/ADTSECT.HTML (state 2014),
 *              and the Linux kernel patches supplied by IBM (mdisk).
 *
 *              (2014) at: http://www.vm.ibm.com/pubs/cms630/FSTD.HTML
 *              (2014) at: http://www.vm.ibm.com/pubs/cms630/ADTSECT.HTML
 *
 *              Some specifications contained here were taken from
 *              other filesystem drivers found in the Linux source tree.
 *
 *              Use 'iconv -f iso8859-1' to read this file on Linux.
 *
 */

#define         CMSFS_AUTHOR            "Rick Troth <rmt@casita.net>"
#define         CMSFS_DESCRIPTION       "CMS Minidisk Filesystem Support"
#define         CMSFS_VERSION           "1.1.11"
#define         CMSFS_DATE              "2023-09-05"   /* 2023-Sep-05 */
/*        Note! The "v" and "r" of the VRM should match               *
 *              between this header and the RPM "spec" file.          *
 *              The "m" (VRM) and the "release" (RPM) might not.      */

/* ----------------------------------------------------------- Constants
 *  Constants used by the CMS FS driver and utility.
 *  Most of these are beyond the low-order byte to avoid collisions
 *  though they are not intended for shared use with other constants.
 */
/*  the following are for the flags member of the CMSINODE struct  */
#define         CMSFSANY        0x0000
#define         CMSFSBIN        0x0100
#define         CMSFSTXT        0x0200
#define         CMSFSRAW        0x0300
#define         CMSFSEXE        0x1000

/*
                CMSFSANY - default
                CMSFSBIN - no translation, ignore record boundaries
                CMSFSTXT - A/E translation, NL at record boundaries
                CMSFSRAW - two-byte length followed by untrans record
                CMSFSEXE - execute bit set when copied (util) or mounted
 */

/*  the following are for the flags member of the CMSSUPER struct  */
#define         CMSFSFBA        0x0400
#define         CMSFSCKD        0x0800

/*  error codes,  should match errno.h as closely as possible  */
#define         CMSFS_ENOENT    2
#define         CMSFS_EIO       5
#define         CMSFS_EFAULT    14
#define         CMSFS_EBUSY     16
#define         CMSFS_EINTERNAL 77
#define         CMSFS_ENOCMS    123
#define         CMSFS_ENOTCMS   124
/*  AC removed the above constants  */
/*  RMT will remove them permanently in v2  */

/* ------------------------------------------------------------ CMSFSBKT
 *  CMS FS blocksize test array
 *  possible block sizes for a CMS "EDF" minidisk filesystem
 *  Should be in ascending order so that the volume type identifier
 *  (see CMSFSVSK)  is not incorrectly found in a file in the FS.
 */
#define   CMSFSBKT      { 512 , 1024 , 2048 , 4096 , 0 }
/*  the logic will expect last item in array to be zero  */

/* ------------------------------------------------------------ CMSFSVSK
 *  CMS FS volser key or magic number;  always EBCDIC for "CMS1"
 *  four bytes  (not NULL terminated when read from disk)
 *      It would be simply a 32-bit magic number,  0xC3D4E2F1,
 *      except that we want to be endian-ness immune
 *      and it really is a character string,  not an integer.
 */
#define   CMSFSVSK      { 0xC3 , 0xD4 , 0xE2 , 0xF1 , 0 }
/*  logic will expect last item in array to be zero  */

/* ------------------------------------------------------------ CMSFSFST
 *  C-flavored FSTD for the CMS FS for Linux.
 *    Based on: http://www.vm.ibm.com/pubs/cmsdacb/FSTD.HTML
 *  I did not bother with the FSTDFNFT union to avoid clutter.
 *  If someone wants it,  feel free to add it here.
 */
typedef struct  CMSFSFST   /*  File Status Table DSECT  */
{
unsigned char FSTFNAME[8] ;      /* filename */
unsigned char FSTFTYPE[8] ;      /* filetype */
unsigned char FSTDATEW[2] ;      /* DATE LAST WRITTEN - MMDD */
unsigned char FSTTIMEW[2] ;      /* TIME LAST WRITTEN - HHMM */
unsigned char FSTWRPNT[2] ;      /* WRITE POINTER - ITEM NUMBER */
unsigned char FSTRDPNT[2] ;      /* READ POINTER - ITEM NUMBER */
unsigned char FSTFMODE[2] ;      /* FILE MODE - LETTER AND NUMBER */
unsigned char FSTRECCT[2] ;      /* NUMBER OF LOGICAL RECORDS */
unsigned char FSTFCLPT[2] ;      /* FIRST CHAIN LINK POINTER */
unsigned char FSTRECFM[1] ;      /* F*1 - RECORD FORMAT - F OR V */
#define         FSTDFIX         0xC6    /* C'F' - Fixed record format */
#define       FSTDVAR       0xE5    /* C'V' - Variable record format */
unsigned char FSTFLAGS[1] ;      /* F*2 - FST FLAG BYTE */
#define     FSTRWDSK       0x80 /* READ/WRITE DISK */
#define     FSTRODSK       0x00 /* READ/ONLY DISK */
#define     FSTDSFS        0x10 /* Shared File FST */
#define     FSTXRDSK       0x40 /* EXTENSION OF R/O DISK */
#define     FSTXWDSK       0xC0 /* EXTENSION OF R/W DISK */
#define     FSTEPL         0x20 /* EXTENDED PLIST */
#define     FSTDIA         0x40 /* ITEM AVAILABLE */
#define     FSTDRA         0x01 /* PREVIOUS RECORD NULL */
#define     FSTCNTRY       0x08 /* Century for date last
                                written (0=19, 1=20), corresponds
                                to FSTYEARW, FSTADATI. */
#define     FSTACTRD       0x04 /* ACTIVE FOR READING */
#define     FSTACTWR       0x02 /* ACTIVE FOR WRITING */
#define     FSTACTPT       0x01 /* ACTIVE FROM A POINT */
#define     FSTFILEA       0x07 /* THE FILE IS ACTIVE */
unsigned char FSTLRECL[4] ;      /* LOGICAL RECORD LENGTH */
unsigned char FSTBLKCT[2] ;      /* NUMBER OF 800 BYTE BLOCKS */
unsigned char FSTYEARW[2] ;      /* YEAR LAST WRITTEN */
unsigned char FSTFOP[4] ;        /* ALT. FILE ORIGIN POINTER */
unsigned char FSTADBC[4] ;       /* ALT. NUMBER OF DATA BLOCKS */
unsigned char FSTAIC[4] ;        /* ALT. ITEM COUNT */
unsigned char FSTNLVL[1] ;       /* NUMBER OF POINTER BLOCK LEVELS */
unsigned char FSTPTRSZ[1] ;      /* LENGTH OF A POINTER ELEMENT */
unsigned char FSTADATI[6] ;      /* ALT. DATE/TIME(YY MM DD HH MM SS) */
unsigned char FSTREALM[1] ;      /* Real filemode */
unsigned char FSTFLAG2[1] ;      /* F*3 - FST FLAG BYTE 2 FSTFLAG2 */
#define     FSTPIPEU       0x10 /* Reserved for CMS
                                        PIPELINES usage */
unsigned char reserved[2] ;
} FSTD ; /*         File Status Table DSECT  */

#define         FSTDSIZE        0x00000040 /* FST SIZE IN BYTES */

/*
 FSTD Storage Layout*** FSTD - File Status Table DSECT
*
*     +-------------------------------------------------------+
*   0 |                       FSTFNAME                        |
*     +-------------------------------------------------------+
*   8 |                       FSTFTYPE                        |
*     +-------------+-------------+-------------+-------------+
*  10 |  FSTDATEW   |  FSTTIMEW   |  FSTWRPNT   |  FSTRDPNT   |
*     +-------------+-------------+-------------+------+------+
*  18 |  FSTFMODE   |  FSTRECCT   |  FSTFCLPT   |:RECFM|:FLAGS|
*     +-------------+-------------+-------------+------+------+
*  20 |         FSTLRECL          |  FSTBLKCT   |  FSTYEARW   |
*     +---------------------------+-------------+-------------+
*  28 |          FSTFOP           |         FSTADBC           |
*     +---------------------------+------+------+-------------+
*  30 |          FSTAIC           |:NLVL |:PTRSZ|   (036)-    |
*     +---------------------------+------+------+-------------+
*  38 |        -FSTADATI          |:REALM|:FLAG2|/////////////|
*     +---------------------------+------+------+-------------+
*  40
*

This information is based on VM/ESA 2.4.0.
Last updated on 12 Aug 1999 at 11:21:05 EDT.
Copyright IBM Corporation, 1990, 1999

 */

/* ------------------------------------------------------------ CMSFSADT
 *  Mock ADTSECT for the CMS FS for Linux.
 *    Based on: http://www.vm.ibm.com/pubs/cmsdacb/ADTSECT.HTML
 *              noting offset 144 (0x90) for 80 bytes (0x50)
 */
typedef struct  CMSFSADT
{
unsigned char ADTIDENT[4] ;   /*  VOL START / LABEL IDENTIFIER  */
unsigned char ADTID[6] ;      /*  VOL START / VOL IDENTIFIER  */
unsigned char ADTVER[2] ;     /*  VERSION LEVEL  */
unsigned char ADTDBSIZ[4] ;   /*  DISK BLOCK SIZE  */
unsigned char ADTDOP[4] ;     /*  DISK ORIGIN POINTER  */
unsigned char ADTCYL[4] ;     /*  NUM OF FORMATTED CYL ON DISK  */
unsigned char ADTMCYL[4] ;    /*  MAX NUM FORMATTED CYL ON DISK  */
unsigned char ADTNUM[4] ;     /*  Number of Blocks on disk  */
unsigned char ADTUSED[4] ;    /*  Number of Blocks used  */
unsigned char ADTFSTSZ[4] ;   /*  SIZE OF FST  */
unsigned char ADTNFST[4] ;    /*  NUMBER OF FST'S PER BLOCK  */
unsigned char ADTDCRED[6] ;   /*  DISK CREATION DATE (YYMMDDHHMMSS)  */
unsigned char ADTFLGL[1] ;    /*  LABEL FLAG BYTE (ADTFLGL)  */
#define         ADTCNTRY        0x01    /*  Century for disk creation
                                        date (0=19, 1=20), corresp to
                                        ADTDCRED.  */
unsigned char reserved[1] ;
unsigned char ADTOFFST[4] ;   /*  DISK OFFSET WHEN RESERVED  */
unsigned char ADTAMNB[4] ;    /*  ALLOC MAP BLOCK WITH NEXT HOLE  */
unsigned char ADTAMND[4] ;    /*  DISP INTO HBLK DATA OF NEXT HOLE  */
unsigned char ADTAMUP[4] ;    /*  DISP INTO USER PART OF ALLOC MAP  */
unsigned char ADTOFCNT[4] ;   /*  ct of SFS open files for this ADT  */
unsigned char ADTSFNAM[8] ;   /*  NAME OF SHARED SEGMENT  */
} CMSFSADT ;

#define       ADTLABSZ      0x00000050  /*  LENGTH OF THE LABEL PART  */

/*
     MAPPING OF VOLUME LABEL
0090  144 Dbl-Word     8 * (0)
0090  144 Character    4 ADTIDENT       VOL START / LABEL IDENTIFIER
0094  148 Character    6 ADTID          VOL START / VOL IDENTIFIER
009A  154 Character    2 ADTVER         VERSION LEVEL
009C  156 Signed       4 ADTDBSIZ       DISK BLOCK SIZE
00A0  160 Signed       4 ADTDOP         DISK ORIGIN POINTER
00A4  164 Signed       4 ADTCYL         NUM OF FORMATTED CYL ON DISK
00A8  168 Signed       4 ADTMCYL        MAX NUM FORMATTED CYL ON DISK
00AC  172 Signed       4 ADTNUM         Number of Blocks assigned to the
                                        minidisk or, in the case of an
                                        SFS top directory, the number of
                                       blocks assigned to the containing
                                        filespace. This field is not set
                                        for SFS subdirectories except as
                                        a byproduct of the ADT Lookup
                                       function, which queries the space
                                        values for subdirs and also
                                        leaves these values in the ADT
                                        space fields.
00B0  176 Signed       4 ADTUSED        Number of Blocks used in the
                                        minidisk or, in the case of an
                                        SFS top directory, the number of
                                       blocks consumed in the containing
                                        filespace. This field is not set
                                        for SFS subdirectories except as
                                        a byproduct of the ADT Lookup
                                       function, which queries the space
                                        values for subdirs and also
                                        leaves these values in the ADT
                                        space fields.
00B4  180 Signed       4 ADTFSTSZ       SIZE OF FST
00B8  184 Signed       4 ADTNFST        NUMBER OF FST'S PER BLOCK
00BC  188 Character    6 ADTDCRED      DISK CREATION DATE (YYMMDDHHMMSS)
00C2  194 Bitstring    1 ADTFLGL        LABEL FLAG BYTE (ADTFLGL)
     EQUATES FOR ADT LABEL FLAG BYTE (ADTFLGL)
          .... ...1      ADTCNTRY       X'01' Century for disk creation
                                        date (0=19, 1=20), corresp to
                                        ADTDCRED.
00C3  195 Character    1 *              RESERVED
00C4  196 Signed       4 ADTOFFST       DISK OFFSET WHEN RESERVED
00C8  200 Signed       4 ADTAMNB        ALLOC MAP BLOCK WITH NEXT HOLE
00CC  204 Signed       4 ADTAMND        DISP INTO HBLK DATA OF NEXT HOLE
00D0  208 Signed       4 ADTAMUP        DISP INTO USER PART OF ALLOC MAP
00D4  212 Signed       4 ADTOFCNT       Count of SFS open files for this
                                        ADT NOTE: ADTOFCNT IS NOT REALLY
                                        PART OF THE VOLUME LABEL - IT IS
                                        NOT USED FOR MINIDISKS.
00D8  216 Character    8 ADTSFNAM       NAME OF SHARED SEGMENT
          00000050       ADTLABSZ       *-ADTIDENT LENGTH OF THE LABEL
                                        PORTION

*     +---------------------------+---------------------------+
*  90 |         ADTIDENT          |          ADTID-           |
*     +-------------+-------------+---------------------------+
*  98 |   -(094)    |   ADTVER    |         ADTDBSIZ          |
*     +-------------+-------------+---------------------------+
*  A0 |          ADTDOP           |          ADTCYL           |
*     +---------------------------+---------------------------+
*  A8 |         ADTMCYL           |          ADTNUM           |
*     +---------------------------+---------------------------+
*  B0 |         ADTUSED           |         ADTFSTSZ          |
*     +---------------------------+---------------------------+
*  B8 |         ADTNFST           |        ADTDCRED-          |
*     +-------------+------+------+---------------------------+
*  C0 |   -(0BC)    |:FLGL |//////|         ADTOFFST          |
*     +-------------+------+------+---------------------------+
*  C8 |         ADTAMNB           |         ADTAMND           |
*     +---------------------------+---------------------------+
*  D0 |         ADTAMUP           |         ADTOFCNT          |
*     +---------------------------+---------------------------+
*  D8 |                       ADTSFNAM                        |
*     +-------------------------------------------------------+
*  E0
*

This information is based on VM/ESA 2.4.0.
Last updated on 12 Aug 1999 at 11:17:01 EDT.
Copyright IBM Corporation, 1990, 1999

 */

/* ------------------------------------------------------------ CMSFSEXT
 *  A structure for the file extension mapping table.
 *  Used for handling default translation based on filetype.
 *  The ftype (filetype) is actually only eight bytes,
 *  plus one for the string terminator,  but then
 *  plus one more for easier alignment.
 */
typedef struct  CMSFSEXT  {
        unsigned  char      ftype[10];
        unsigned short int  flags;
/*      u16 flags;  AC used "u16" for Linux kernel  */
                          }  CMSFSEXT ;

/* ------------------------------------------------------------ CMSINODE
 *  An inode structure (non VFS) for the CMS FS driver and utility.
 *  This is used stand-alone by the utility,
 *  and used in concert with the VFS inode structure by the driver.
 */
typedef struct  CMSINODE  {

long int magic;         /*  not used at this time  */
void   *vfsinode;       /*  pointer to Linux VFS inode, if any  */
unsigned char name[18] , fname[9], ftype[9], fmode[3];
int     lrecl ;
unsigned char recfm[2] ;
int     index ;         /*  index into directory of this file  */
                        /*  (also serves as an inode number)  */
time_t  ctime ;         /*  computed Unix time from CMS timestamp  */
int     bloks ;         /*  blocks in first-level  */
                        /*  If there is only one level,
                            then this is the total number of blocks  */
int     items ;         /*  number of records in this file  */
int     origin;         /* "base one" File Origin Pointer (FOP) */
                        /* "base zero" offset is derived from FOP */

int     level ;         /*  level of indirection  */
int     psize ;         /*  size of indir pointers  */
int     bytes ;         /*  Not "size",  because the term is
                            ambiguous for IBM files which might have
                            "sizes" stated in terms of records, blocks,
                            tracks, or possibly even cylinders.
                            Must be computed, and recomputed ...  */

int     rdpnt;          /*  the current record  */
int     rdblk;          /*  the current block  */
int     rdoff;          /*  the offset into the current block  */
void   *rdbuf;          /*  buffer storage for current block  */

int     rdoff2;         /*  the offset into the current record  */
void   *rdbuf2;         /*  buffer storage for current record  */

unsigned
int     flags ;         /*  V or F bit, binary or text bit, etc  */
                        /*  CAREFUL HOW MUCH YOU PACK IN HERE.
                            This structure is intended for portability;
                            this field might not always be 32 bits!!  */
struct
CMSSUPER  *cmssuper;    /*  the containing CMS superblock  */
int     error;          /* error code, if any */
                          } /* (AC removed) */ CMSINODE ;

/* for AC */
#define cms_inode CMSINODE
/* (RMT will change globally in v2) */

/* ------------------------------------------------------------ CMSSUPER
 *  A filesystem info structure for the CMS FS driver and utility.
 *  AKA: "superblock for CMS",  therefore FKA: "CMSVOLID"
 */
typedef struct  CMSSUPER  {

long int magic;         /*  not used at this time  */
void   *vfssuper;       /*  pointer to Linux VFS superblock, if any  */
unsigned char vtype[5];       /* volume type ID; "CMS1" for CMS vols */
unsigned char volid[7];       /* volume label (volume serial number) */
long
int     blksz;          /* statfs f_bsize
                           block size of this volume */
int     pbksz;          /*  physical blocksize  */
int     origin;         /* "base one" Directory Origin Pointer (DOP) */
                        /* "base zero" offset is derived from DOP */
int     ncyls;          /* number of cylinders used by filesystem */
int     mcyls;          /* number of cylinders on the disk */
                        /* CMS has a concept of "recomp" where
                           cylinders beyond the filesystem may be
                           used by other things, like a boot loader. */
long
int     blocks;         /* statfs f_blocks
                           total number of blocks in the filesystem */
long
int     bkused;         /* blocks used (per ADT) */
long
int     bkfree;         /* statfs f_bfree
                           blocks free (computed) */
int     fstsz;          /* size of each directory entry (of each FST) */
int     fstct;          /* number of FSTs (FST count) per block */
time_t  ctime;          /* time when this filesystem was created */
int     resoff;         /* reserved offset */
                        /* CMS has a concept of a "reserved disk"
                           where one file occupies the entire disk.
                           Additionally,  that file is ref'd by this
                           so that the filesystem (directory) mechanism
                           can be bypassed. */
int     devfd;          /* when accessed from "user space"
                           (when run as a utility, not the driver),
                            the file descriptor of the file
                            containg this CMS filesystem  */
int     devid;          /* the Linux device ID, if available */
int     devno;          /* the S/390 device number, if available */
int     subch;          /* the S/390 sub-channel, if available */
long
int     files;          /* statfs f_files
                           extracted from directory FST "items" */
int     inuse;          /* number of files open at any time */
                        /*  (map to ADTOFCNT, perhaps)  */
unsigned
int     flags;          /*  FBA bit, and so forth */
                        /*  CAREFUL HOW MUCH YOU PACK IN HERE.
                            This structure is intended for portability;
                            this field might not always be 32 bits!!  */
struct
CMSINODE  *cmsrooti;    /*  pointer to directory inode (CMS flavor)  */
                        /*  (member 'cmssuper' must point back here)  */
struct
CMSFSEXT  *cmsfsext;    /*  pointer to exten map (type map) struct  */
int     error;          /* error code, if any */
                          } /* (AC removed) */ CMSSUPER ;

/* for AC */
#define cms_super CMSSUPER
/* (RMT will change globally in v2) */


/*
        The following struct taken from /usr/include/time.h:

   Copyright (C) 1991,92,93,94,95,96,97,98,99 Free Software Foundation
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifdef          CMSFS_NEED_TM
/*  we wonder about chaining off of  #ifdef __KERNEL__  instead  */
struct cmsfs_tm
{
  int tm_sec;                   /* Seconds.     [0-60] (1 leap sec) */
  int tm_min;                   /* Minutes.     [0-59] */
  int tm_hour;                  /* Hours.       [0-23] */
  int tm_mday;                  /* Day.         [1-31] */
  int tm_mon;                   /* Month.       [0-11] */
  int tm_year;                  /* Year - 1900.  */
  int tm_wday;                  /* Day of week. [0-6] */
  int tm_yday;                  /* Days in year.[0-365] */
  int tm_isdst;                 /* DST.         [-1/0/1]*/
  long int tm_gmtoff;           /* Seconds east of UTC.  */
/* __const char *tm_zone; */
  unsigned char *tm_zone;        /* Timezone abbreviation.  */
};
#else
#define         cmsfs_tm                tm
#endif


/* ------------------------------------------------------------------ *
 *  Function Prototypes                                               *
 * ------------------------------------------------------------------ */

/* ------------------------------------------------------------------ *
 *  Function Prototypes used by all CMS FS implementations            *
 * ------------------------------------------------------------------ */
void cmsfs_error(unsigned char *);
int cmsfs_bread(struct CMSSUPER *,void *,int,int);
void *cmsfs_malloc(int);
void cmsfs_free(void *);

int cmsfs_map_ADT(struct CMSSUPER *);
int cmsfs_map_FST(struct CMSINODE *,struct CMSFSFST *);
void cmsfs_map_EXT(struct CMSINODE *);
struct CMSINODE * cmsfs_lookup(struct CMSINODE *,unsigned char *);
ssize_t cmsfs_read(struct CMSINODE *,void *,size_t);

/* ------------------------------------------------------------------ *
 *  Function Prototypes used by the utility                           *
 * ------------------------------------------------------------------ */
struct CMSSUPER * cmsfs_vopen(unsigned char *);
int cmsfs_vclose(struct CMSSUPER *);
struct CMSINODE * cmsfs_fopen(unsigned char *,struct CMSSUPER *,int);
int cmsfs_fclose(struct CMSINODE *);
int cmsfs_fread(void *,int,struct CMSINODE *);

/*  simple open(), read(), close() semantics  */
struct CMSINODE * cmsfs_open(unsigned char *,int,unsigned char *);
int cmsfs_close(struct CMSINODE *);

/*
cmsfs_feof(),
cmsfs_ferror(),
cmsfs_write() ... NOT!
 */

void cmsfs_xdump(unsigned char *,int);

/* ------------------------------------------------------------------ *
 *  Function Prototypes used by the driver are presently              *
 *  all contained within one compilation unit.                        *
 * ------------------------------------------------------------------ */

/* ------------------------------------------------------------------ *
 *  Global Static (tough on radio reception, eh?)                     *
 * ------------------------------------------------------------------ */
static unsigned char cmsfs_ermsg[256];




