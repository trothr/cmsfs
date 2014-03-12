/*
 *
 *        Name: cmsfsany.c (C program source)
 *              CMS FS common routines (any mode: util or driver)
 *              This is included in cmsfsusa.c and cmsfsvfs.c.
 *
 *    See also: cmsfsusa.c (utility) and cmsfsvfs.c (driver)
 *
 *              This source contains the routines common to both the
 *              "utility" build of CMS FS and the "driver" build,
 *              to be included by either "cmsfsvfs.c" or "cmsfsusa.c".
 *              If there were ever a stand alone mode (neither Linux
 *              filesystem nor a Unix utility),  then these routines
 *              should be common to that mode also.
 *
 */

/* ------------------------------------------------------------ CMSFSBEX
 *  This function was moved to cmsfsusa.c (the utility source).
 *  FS driver should use kernel header macros for endian conversion.
 */

/* ------------------------------------------------------------ CMSFSX2I
 *  CMS FS heXadecimal-to-Integer function
 *  The EDF filesystem uses several "packed decimal" fields.
 *  (Actually a visual approximation of packed decimal format
 *  lacking the sign nibble.)   This is for converting them to integers.
 */
int cmsfsx2i ( unsigned char *ivalue , int l )
  {
    int         i , ovalue ;
    ovalue = (ivalue[0] & 0x0F) + ((ivalue[0]>>4) & 0x0F) * 10;
    for (i = 1 ; i < l ; i++)
      {
        ovalue = ovalue << 8 ;
        ovalue += (ivalue[i] & 0x0F) + ((ivalue[i]>>4) & 0x0F) * 10;
      }
    return ovalue ;
  }

/* ---------------------------------------------------- CMSFS_FIND_LABEL
 *  Look for the CMS volume label structure (ADT).
 *  If we don't find it,  this ain't no CMS EDF filesystem.
 *  Returns the physical blocksize of the disk on success.
 */
int cmsfs_find_label(struct CMSSUPER *vol,struct CMSFSADT *adt)
  {
    int         i, rc;
    static
    unsigned  char  cmsfsflb[4096];     /*  known max blocksize  */
                                       /*  (probably should compute)  */

    /*  char array (4 bytes) "CMS1" filesystem identifier  */
    unsigned
    char        cmsfsvsk[8]     =       CMSFSVSK ;
    /*  (a "magic number" by any other name)  */

    /*  array of possible CMS FS blocksizes  */
    int         cmsfsbkt[8]     =       CMSFSBKT ;

#ifdef  CMSFS_DEBUG
    (void) sprintf((char*)cmsfs_ermsg,
    "cmsfs_find_label(0x%08X,0x%08X)",vol,adt);
    cmsfs_error(cmsfs_ermsg);
#endif

    /*
     *  FBA DASDs are special.  Physical blocsize is always 512
     *  and the label is at physical offset 512 (second record),
     *  though the logical blocksize may be 512, 1K, 2K, or 4K.
     */
    if (vol->pbksz == 0 || vol->pbksz == 512) {
#ifdef  CMSFS_DEBUG
        cmsfs_error(
            (unsigned char *)
            "cmsfs_find_label(): trying FBA ...");
#endif  /* CMSFS_DEBUG */
    /*  read FBA block #1 (second record)  */
    rc = cmsfs_bread(vol,cmsfsflb,1,512);
    if (rc != 512)
      {
        if (vol->error == 0)
        vol->error = CMSFS_EIO;         /* I/O error */
        return -1;
      }
    (void) memcpy(adt,cmsfsflb,sizeof(*adt));

    /*  check for the CMS1 magic at the FBA offset  */
    if (adt->ADTIDENT[0] == cmsfsvsk[0]
     && adt->ADTIDENT[1] == cmsfsvsk[1]
     && adt->ADTIDENT[2] == cmsfsvsk[2]
     && adt->ADTIDENT[3] == cmsfsvsk[3])
      {
        int     blksz;
/*      blksz = cmsfsbex((unsigned char *)adt->ADTDBSIZ,4);           */
        blksz = be32_to_cpu(*(int*)adt->ADTDBSIZ);
        if (blksz != 512)
          {
            (void) sprintf((char*)cmsfs_ermsg,
 "cmsfs_find_label(): FS blksize %d does not match device blksize %d",
                        blksz,512);
            cmsfs_error(cmsfs_ermsg);
          }
        vol->flags = CMSFSFBA;
        vol->blksz = 512;
        return vol->blksz;
      } }

    /*  not an FBA volume; try CKD blocksizes  */
    /*
     *  CKD DASDs are C/H/S in nature and can have any blocksize
     *  that the utility or operating system decided to put there.
     *  OS/390 uses no particular blocksize,  referring to tracks
     *  directly.   For CMS CKD volumes,  the physical blocksize
     *  should match logical,  unless obscured by the access method.
     */
    for (i = 0 ; cmsfsbkt[i] != 0 ; i++)
      {
    if (vol->pbksz == 0 || vol->pbksz == cmsfsbkt[i]) {
#ifdef  CMSFS_DEBUG
        (void) sprintf((char*)cmsfs_ermsg,
            "cmsfs_find_label(): trying %d blocksize ...",
            cmsfsbkt[i]);
        cmsfs_error(cmsfs_ermsg);
#endif  /* CMSFS_DEBUG */
        /*  read CKD block #2 (third record)  */
        rc = cmsfs_bread(vol,cmsfsflb,2,cmsfsbkt[i]);
        if (rc != cmsfsbkt[i])
          {
            if (vol->error == 0)
            vol->error = CMSFS_EIO;     /* I/O error */
            return -1;
          }
        (void) memcpy(adt,cmsfsflb,sizeof(*adt));

        /*  check for the CMS1 magic  */
        if (adt->ADTIDENT[0] == cmsfsvsk[0]
         && adt->ADTIDENT[1] == cmsfsvsk[1]
         && adt->ADTIDENT[2] == cmsfsvsk[2]
         && adt->ADTIDENT[3] == cmsfsvsk[3])
          {
            int     blksz;
/*          blksz = cmsfsbex(adt->ADTDBSIZ,4);                        */
            blksz = be32_to_cpu(*(int*)adt->ADTDBSIZ);

            if (blksz == cmsfsbkt[i])
              {
                vol->flags = CMSFSCKD;
                vol->blksz = cmsfsbkt[i];
                return vol->blksz;
              }
            (void) sprintf((char*)cmsfs_ermsg,
 "cmsfs_find_label(): FS blksize %d does not match device blksize %d",
                        blksz,cmsfsbkt[i]);
            cmsfs_error(cmsfs_ermsg);
          } }
      }

    vol->error = CMSFS_ENOCMS;  /* "No medium found" */
    return -1;                  /*  not a CMS volume  */
  }

/*
 *  The above routine presumes that even an FBA volume will be
 *  at least 12K.   This is technically incorrect:  FBA CMS formatting
 *  can use as little as 8K.   But since such a volume would have
 *  only 1024 bytes for storage,  I think this is a small exposure.
 */

/* ------------------------------------------------------- CMSFS_MAP_ADT
 *  Map an ADT struct into our own CMSSUPER superblock struct.
 *  CMS flavor superblock struct must be supplied.
 *  CMS flavor inode struct will be allocated,  if this succeeeds.
 */
int cmsfs_map_ADT(struct CMSSUPER *vol)
  {
    int         i,      /*  a counter  */
                rc,     /*  a return code  */
                bktry;  /*  a trial blocksize  */

    /*  partial "ADT" structure (per IBM)  */
    static
    struct      CMSFSADT        cmsvol;

    static
    unsigned  char  cmsfsflb[4096];     /*  known max blocksize  */
    /*  (probably should compute instead of always using max)  */

#ifdef  CMSFS_DEBUG
    (void) sprintf((char*)cmsfs_ermsg,"cmsfs_map_ADT(0x%08X)",vol);
    cmsfs_error(cmsfs_ermsg);
#endif

    /*  should check that CMSSUPER passed is non-NULL  */
    /*  should perhaps check that vol->inuse == 0  */

    /*  set this to zero now because it serves as a check later  */
    vol->blksz = 0;

    /*  look for the CMS volume label (aka VOLSER)  */
    rc = cmsfs_find_label(vol,&cmsvol);
    /*  this also effects a load of the ADT struct to &cmsvol  */
    /*  and sets the blksz member to match the volume found  */

    /*  did we find a CMS1 volume label?  */
    if (rc <= 0)
      {
        cmsfs_error(
            (unsigned char*)
            "cmsfs_map_ADT(): cannot find a CMS1 label");
        return -1;
      }

    /*  extract volume label and translate  */
    for (i = 0 ; i < 6 ; i++) vol->volid[i] = cmsvol.ADTID[i] ;
    vol->volid[6] = 0x00;
#ifdef  CMSFS_HOST_ASCII
    (void) stretoa(vol->volid);
#endif

#ifdef  CMSFS_DEBUG
    (void) sprintf((char*)cmsfs_ermsg,
    "cmsfs_map_ADT(): processing volume '%s'",vol->volid);
    cmsfs_error(cmsfs_ermsg);
#endif

    /*  extract "directory origin pointer"  */
/*  vol->origin = cmsfsbex(cmsvol.ADTDOP,4);                          */
    vol->origin = be32_to_cpu(*(int*)cmsvol.ADTDOP);

    /*  extract number of cylinders used  */
/*  vol->ncyls = cmsfsbex(cmsvol.ADTCYL,4);                           */
    vol->ncyls = be32_to_cpu(*(int*)cmsvol.ADTCYL);

    /*  extract max number of cylinders  */
/*  vol->mcyls = cmsfsbex(cmsvol.ADTMCYL,4);                          */
    vol->mcyls = be32_to_cpu(*(int*)cmsvol.ADTMCYL);

    /*  extract "total blocks on disk" count  */
/*  vol->blocks = cmsfsbex(cmsvol.ADTNUM,4);                          */
    vol->blocks = be32_to_cpu(*(int*)cmsvol.ADTNUM);

    /*  compute "blocks used" count  */
/*  vol->bkused = cmsfsbex(cmsvol.ADTUSED,4);                         */
    vol->bkused = be32_to_cpu(*(int*)cmsvol.ADTUSED);
    vol->bkused += 1 ;  /*  why???  */

    /*  compute "blocks free" count  */
    vol->bkfree = vol->blocks - vol->bkused ;

    /*  compute time (as ctime) when this volume was created  */
      {
        struct cmsfs_tm temptime;     /*  temporary  */
        temptime.tm_sec = cmsfsx2i(
            (unsigned char *)&cmsvol.ADTDCRED[5],1);
        temptime.tm_min = cmsfsx2i(
            (unsigned char *)&cmsvol.ADTDCRED[4],1);
        temptime.tm_hour = cmsfsx2i(
            (unsigned char *)&cmsvol.ADTDCRED[3],1);
        temptime.tm_mday = cmsfsx2i(
            (unsigned char *)&cmsvol.ADTDCRED[2],1);
        temptime.tm_mon = cmsfsx2i(
            (unsigned char *)&cmsvol.ADTDCRED[1],1) - 1;
        temptime.tm_year = cmsfsx2i(
            (unsigned char *)&cmsvol.ADTDCRED[0],1);
        if (cmsvol.ADTFLGL[0] & ADTCNTRY)
                temptime.tm_year += 100 ;
    vol->ctime = cmsfs_mktime(&temptime);
      }

    /*  extract offset to reserved file,  if any  */
/*  vol->resoff = cmsfsbex(cmsvol.ADTOFFST,4);                        */
    vol->resoff = be32_to_cpu(*(int*)cmsvol.ADTOFFST);

    /*  extract size and number of FSTs  */
/*  vol->fstsz = cmsfsbex(cmsvol.ADTFSTSZ,4);                         */
    vol->fstsz = be32_to_cpu(*(int*)cmsvol.ADTFSTSZ);
/*  vol->fstct = cmsfsbex(cmsvol.ADTNFST,4);                          */
    vol->fstct = be32_to_cpu(*(int*)cmsvol.ADTNFST);

    /*  initial filetype mapping table is NULL  */
    vol->cmsfsext = NULL;

    /*  allocate and map the directory inode from its FST  */
    vol->cmsrooti = cmsfs_malloc(sizeof(struct CMSINODE));
    if (vol->cmsrooti == NULL)
      {
        cmsfs_error(
            (unsigned char *)
            "cmsfs_map_ADT(): cannot allocate CMS directory inode");
        return -1;
      }

    /*  read in the first FST  */
    rc = cmsfs_bread(vol,cmsfsflb,vol->origin-1,vol->blksz);
    if (rc != vol->blksz)
      {
        cmsfs_free(vol->cmsrooti);
        vol->inuse = 0;         /*  we just deallocated that inode  */
        return -1;
      }

    vol->inuse = 1;             /*  we just allocated one inode  */

    /*  point dir inode back to the superblock  */
    vol->cmsrooti->cmssuper = vol;
    /*  must be set BEFORE calling cmsfs_map_FST()  */

    /*  map the directory ADT into a the root directory inode  */
    rc = cmsfs_map_FST(vol->cmsrooti,(struct CMSFSFST *)cmsfsflb);
    if (rc != 0)  {
        (void) sprintf((char*)cmsfs_ermsg,
"cmsfs_map_ADT(): cmsfs_map_FST() of the directory returned %d",rc);
        cmsfs_error(cmsfs_ermsg);
        cmsfs_free(vol->cmsrooti);
        vol->inuse = 0;         /*  we just deallocated that inode  */
        return rc; }

    vol->cmsrooti->flags = CMSFSBIN;    /*  directory is F 64 binary  */

    /*  (announce if debugging) dir inode is now allocd and mapped  */
#ifdef  CMSFS_DEBUG
    cmsfs_error(
        (unsigned char *)
        "cmsfs_map_ADT(): CMS dir inode allocated and mapped");
#endif

    /*  sanity check:  FOP in dir entry must match DOP in vol  */
/*
    if (vol->origin != vol->cmsrooti->origin)
      {
        unsigned char buff[32];
        unsigned char buf2[4096];
        (void) sprintf((char*)cmsfs_ermsg,
 "cmsfs_map_ADT(): FOP %d (of directory) does not match DOP %d",
                vol->cmsrooti->origin,vol->origin);
        cmsfs_error(cmsfs_ermsg);
        (void) sprintf((char*)cmsfs_ermsg,
            "cmsfs_map_ADT(): blocksize %d, levels %d, blocks %d",
                vol->blksz,vol->cmsrooti->level,vol->cmsrooti->bloks);
        cmsfs_error(cmsfs_ermsg);
        (void) lseek(vol->devfd,vol->blksz*(vol->origin-1),SEEK_SET);
        (void) read(vol->devfd,buff,32);
        (void) cmsfsdmp(buff,32);
        (void) lseek(vol->devfd,vol->blksz*(vol->origin-1),SEEK_SET);
        (void) read(vol->devfd,buff,32);
        (void) cmsfsdmp(buff,32);
        cmsfs_error("cmsfs_map_ADT(): Does it look like indirection?");
        (void) cmsfsrd2(vol->cmsrooti,buf2,0);
        (void) cmsfsdmp(buf2,32);
      }
SHOULD RE-CHECK AFTER READING DIRECTORY BLOCKS
 */

    /*  sanity check:  RECFM of a directory must be "F"  */
    if (vol->cmsrooti->recfm[0] != 'F')
      {
        (void) sprintf((char*)cmsfs_ermsg,
            "cmsfs_map_ADT(): directory RECFM '%s' not 'F'",
            vol->cmsrooti->recfm);
        cmsfs_error(cmsfs_ermsg);
        cmsfs_free(vol->cmsrooti);
        vol->inuse = 0;         /*  we just deallocated that inode  */
        return -1;
      }

    /*  sanity check:  LRECL of a directory must be 64  */
    if (vol->cmsrooti->lrecl != 64)
      {
        (void) sprintf((char*)cmsfs_ermsg,
            "cmsfs_map_ADT(): directory LRECL %d not 64",
            vol->cmsrooti->lrecl);
        cmsfs_error(cmsfs_ermsg);
        cmsfs_free(vol->cmsrooti);
        vol->inuse = 0;         /*  we just deallocated that inode  */
        return -1;
      }

    vol->files = vol->cmsrooti->items;

    /*  report  */
#ifdef  CMSFS_DEBUG
    (void) sprintf((char*)cmsfs_ermsg,
        "cmsfs_map_ADT(): volume '%s' blocksize %d files %d",
        vol->volid,vol->blksz,vol->files);
    cmsfs_error(cmsfs_ermsg);
#endif

    vol->error = 0;     /*  no error yet;  this is a new superblock  */

    return 0;
  }

/* ------------------------------------------------------- CMSFS_MAP_FST
 *  Map a CMS FS FST structure to our own CMSINODE "inode" structure.
 *  Operation is function(target,source).
 */
int cmsfs_map_FST(struct CMSINODE *finode , struct CMSFSFST *cmsfst)
  {
    int         i;
    unsigned
    char       *p, *q, *qq;

#ifdef  CMSFS_DEBUG
    (void) sprintf((char*)cmsfs_ermsg,
    "cmsfs_map_FST(0x%08X,0x%08X)",finode,cmsfst);
    cmsfs_error(cmsfs_ermsg);
#endif

    /*  extract and translate filename  */
    p = cmsfst->FSTFNAME;
    q = finode->name;
    qq = finode->fname;
    i = 0;
    while (*p != 0x40 && *p != 0x00 && i < 8)
      {
#ifdef  CMSFS_HOST_ASCII
        *qq = *q = chretoa(*p);
#else
        *qq = *q = *p;
#endif
        if (isupper(*q)) *q = tolower(*q);                  /* LOCASE */
        qq++;  q++;  p++;  i++;
      }
    *q++ = '.';
    *qq++ = 0x00;
    p = cmsfst->FSTFTYPE;
    qq = finode->ftype;
    i = 0;
    while (*p != 0x40 && *p != 0x00 && i < 8)
      {
#ifdef  CMSFS_HOST_ASCII
        *qq = *q = chretoa(*p);
#else
        *qq = *q = *p;
#endif
        if (isupper(*q)) *q = tolower(*q);                  /* LOCASE */
        qq++;  q++;  p++;  i++;
      }
    *q++ = 0x00;
    *qq++ = 0x00;

    /*  extract and translate FMODE  */
    p = cmsfst->FSTFMODE;
    q = finode->fmode;
#ifdef  CMSFS_HOST_ASCII
    q[0] = chretoa(p[0]);  q[1] = chretoa(p[1]);
#else
    q[0] = p[0];  q[1] = p[1];
#endif
    q[2] = 0x00;

    /*  extract and translate RECFM  */
    p = cmsfst->FSTRECFM;
    q = finode->recfm;
#ifdef  CMSFS_HOST_ASCII
    q[0] = chretoa(p[0]);
#else
    q[0] = p[0];
#endif
    q[1] = 0x00;

    /*  extract LRECL  */
/*  finode->lrecl = cmsfsbex(cmsfst->FSTLRECL,4);                     */
    finode->lrecl = be32_to_cpu(*(int*)cmsfst->FSTLRECL);

    /*  extract "directory origin pointer"  */
/*  finode->origin = cmsfsbex(cmsfst->FSTFOP,4);                      */
    finode->origin = be32_to_cpu(*(int*)cmsfst->FSTFOP);

    /*  extract block count  */
/*  finode->bloks = cmsfsbex(cmsfst->FSTADBC,4);                      */
    finode->bloks = be32_to_cpu(*(int*)cmsfst->FSTADBC);

    /*  extract item count  */
/*  finode->items = cmsfsbex(cmsfst->FSTAIC,4);                       */
    finode->items = be32_to_cpu(*(int*)cmsfst->FSTAIC);

    /*  conditionally report these CMS file attributes  */
#ifdef  CMSFS_DEBUG
    (void) sprintf((char*)cmsfs_ermsg,
        "cmsfs_map_FST(): name '%s', mode '%s', recfm '%s', lrecl %d",
        finode->name,finode->fmode,finode->recfm,finode->lrecl);
    cmsfs_error(cmsfs_ermsg);
#endif

    /*  extract date and compute time stamp  */
      {
        struct cmsfs_tm temptime;     /*  temporary  */
        temptime.tm_year = cmsfsx2i(
            (unsigned char *)&cmsfst->FSTADATI[0],1);
        if (cmsfst->FSTFLAGS[0] & FSTCNTRY)
                temptime.tm_year += 100 ;
        temptime.tm_mon = cmsfsx2i(
            (unsigned char *)&cmsfst->FSTADATI[1],1) - 1;
        temptime.tm_mday = cmsfsx2i(
            (unsigned char *)&cmsfst->FSTADATI[2],1);
        temptime.tm_hour = cmsfsx2i(
            (unsigned char *)&cmsfst->FSTADATI[3],1);
        temptime.tm_min = cmsfsx2i(
            (unsigned char *)&cmsfst->FSTADATI[4],1);
        temptime.tm_sec = cmsfsx2i(
            (unsigned char *)&cmsfst->FSTADATI[5],1);
        finode->ctime = cmsfs_mktime(&temptime);
      }

    /*  levels of indirection and pointer size  */
    finode->level = cmsfst->FSTNLVL[0] & 0xFF ;
    finode->psize = cmsfst->FSTPTRSZ[0] & 0xFF ;

    /*  conditionally report these CMS file attributes  */
#ifdef  CMSFS_DEBUG
    (void) sprintf((char*)cmsfs_ermsg,
        "cmsfs_map_FST(): FOP %d, blks, %d recs %d, level %d, psize %d",
        finode->origin,finode->bloks,finode->items,
        finode->level,finode->psize);
    cmsfs_error(cmsfs_ermsg);
#endif

/*
                cmsfst.FSTDATEW[0],cmsfst.FSTDATEW[1],
                cmsfst.FSTTIMEW[0],cmsfst.FSTTIMEW[1],
                cmsfst.FSTYEARW[0],cmsfst.FSTYEARW[1]);
!! above may be all zeros; why? CDF leftover?
*/

/*
nrecs = cmsfst.FSTRECCT[0];
nrecs = nrecs << 8 ;
nrecs += cmsfst.FSTRECCT[1];
(void) printf("%d ",nrecs);
!! above may be zero; why?
 */

    finode->rdpnt =
    finode->rdblk =
    finode->rdoff = 0;          /*  set read pointers to zero  */
    finode->rdoff2 = 0;         /*  set read pointers to zero  */

    finode->rdbuf = NULL;     /*  no work buffer unless allocd later  */
    finode->rdbuf2 = NULL;    /*  no span buffer unless allocd later  */

    finode->error = 0;  /*  no error yet;  this is a new inode  */
    finode->bytes = 0;  /*  don't bother about the size yet  */
    finode->flags = 0x0000;

    return 0;
  }

/* ------------------------------------------------------- CMSFS_MAP_EXT
 *  Map a CMS filetype (sometimes called an "extension").
 *  The terms "filetype" and "extension" are used interchangably here.
 */
#include        "cmsfsext.h"
void cmsfs_map_EXT(struct CMSINODE * fi)
  {
    int     i;
    unsigned char *p;

    /*  sanity check:  be sure supplied "inode" pointer is non-NULL  */
    if (fi == NULL)
      {
        cmsfs_error(
            (unsigned char *)
            "cmsfs_map_EXT(): null CMS inode passed");
        return;
      }

    /*  sanity check:  be sure inode referenced has a superblock  */
    if (fi->cmssuper == NULL)
      {
        cmsfs_error(
            (unsigned char *)
            "cmsfs_map_EXT(): null CMS superblock in CMS inode");
        return;
      }

    /*  silently return if this file has any flags already set  */
    if (fi->flags != 0x0000) return;

    /*  possibly initialize the superblock's filetype mapping table  */
    if (fi->cmssuper->cmsfsext == NULL)
      {
#ifdef  CMSFS_DEBUG
        cmsfs_error(
            (unsigned char *)
            "cmsfs_map_EXT(): initializing filetypes map");
#endif
        fi->cmssuper->cmsfsext = cmsfsext;
        for (i = 0 ; fi->cmssuper->cmsfsext[i].ftype[0] != ' '
            &&       fi->cmssuper->cmsfsext[i].ftype[0] != ' ' ; i++)
          {
            p = fi->cmssuper->cmsfsext[i].ftype;
            while (*p != ' ' && *p != 0x00) p++;
            if (*p == ' ') *p = 0x00;
          }
#ifdef  CMSFS_DEBUG
        (void) sprintf((char*)cmsfs_ermsg,
            "cmsfs_map_EXT(): filetypes map table has %d entries",i);
        cmsfs_error(cmsfs_ermsg);
#endif
      }

    /*  step through the known types list,  looking for a match  */
    for (i = 0 ; fi->cmssuper->cmsfsext[i].ftype[0] != ' ' ; i++)
      {
        if (strncmp(fi->ftype,
            fi->cmssuper->cmsfsext[i].ftype,8) == 0x0000)
          {
            fi->flags = fi->cmssuper->cmsfsext[i].flags;
#ifdef  CMSFS_DEBUG
            (void) sprintf((char*)cmsfs_ermsg,
                    "cmsfs_map_EXT(): found filetype '%s'",
                    fi->cmssuper->cmsfsext[i].ftype);
            cmsfs_error(cmsfs_ermsg);
#endif
            return;
          }
      }

    /*  file is of a type we don't know how to canonicalize  */
    (void) sprintf((char*)cmsfs_ermsg,
        "cmsfs_map_EXT(): filetype '%s' not mapped",fi->ftype);
    cmsfs_error(cmsfs_ermsg);

    return;
  }

/* ------------------------------------------------------------ CMSFSRD2
 *  "Read Function number Two" -- read a block from a CMS file.
 *  This is a second attempt.   THIS FUNCTION NEEDS A BETTER NAME.
 */
int cmsfsrd2(struct CMSINODE *inode,void *buffer,int block)
  {
    int         ppb, i, j, b1, b2, rc;
    unsigned char *bp;

#ifdef  CMSFS_DEBUG
(void) sprintf((char*)cmsfs_ermsg,"cmsfsrd2(,,%d): lev %d, fn '%s'",
                block,inode->level,inode->name);
cmsfs_error(cmsfs_ermsg);
#endif

    /*  when no indirection,  do a simple read  */
    if (inode->level == 0)
    return
         cmsfs_bread(inode->cmssuper,buffer,
        inode->origin-1+block,inode->cmssuper->blksz);

    /* pointers per block */
   if ( inode->psize > 0 ) ppb = inode->cmssuper->blksz / inode->psize ;
                else        ppb = inode->cmssuper->blksz / 4 ;

    /*  read first block for indirection  */
    b1 = block;
    for (i = 0 ; i < inode->level ; i++) b1 = b1 / ppb;
    rc = cmsfs_bread(inode->cmssuper,buffer,
        inode->origin-1+b1,inode->cmssuper->blksz);
    if (rc != inode->cmssuper->blksz)
      {
        (void) sprintf((char*)cmsfs_ermsg,
        "cmsfsrd2(): cmsfs_bread(,,%d,%d) returned %d",
                        inode->origin-1+b1,inode->cmssuper->blksz,rc);
        cmsfs_error(cmsfs_ermsg);
        return rc;
      }

    for (j = 1 ; j <= inode->level ; j++)
      {
        /*  read next block for indirection  */
        b1 = block;
        for (i = j ; i < inode->level ; i++) b1 = b1 / ppb;
        b1 = b1 % ppb;
        bp = buffer;
        bp += b1 * inode->psize;
/*      b2 = cmsfsbex(bp,4);                                          */
        b2 = be32_to_cpu(*(int*)bp);

        /*  If the block pointer is null this is a block of zeros 
         *  and CMS didn't bother to allocate disk space, 
         *  then return a  buffer of zeros.
         */
        if (b2 == 0) {
                memset(buffer, 0, inode->cmssuper->blksz);
                break;
        }
        /*  read next block for indirection  */
        rc = cmsfs_bread(inode->cmssuper,buffer,
                b2-1,inode->cmssuper->blksz);
        if (rc != inode->cmssuper->blksz)
          {
            (void) sprintf((char*)cmsfs_ermsg,
            "cmsfsrd2(): cmsfs_bread(,,%d,%d) returned %d",
                        inode->origin-1+b1,inode->cmssuper->blksz,rc);
            cmsfs_error(cmsfs_ermsg);
            return rc;
          }
      }

    return inode->cmssuper->blksz;
  }

/* -------------------------------------------------------- CMSFS_LOOKUP
 *  Lookup a file in the CMS directory.  Allocates a CMSINODE struct.
 *  After successful call to this function,  the caller might
 *  possibly set the  ->flags  member.  In kernel mode (VFS),
 *  the caller should also set the  ->vfsinode  member.
 */
struct CMSINODE *
cmsfs_lookup(struct CMSINODE * di,unsigned char * fn)
  {
    struct CMSINODE * cmsinode;
    int     i, j, k, rc;
    unsigned char *p, xmatch[18], umatch[18];

#ifdef  CMSFS_DEBUG
    (void) sprintf((char*)cmsfs_ermsg,"cmsfs_lookup(,'%s')",fn);
    cmsfs_error(cmsfs_ermsg);
#endif

    /*  first things first:  confirm superblock and root inode  */
    if (di->cmssuper == NULL)
      {
        cmsfs_error(
            (unsigned char *)
            "cmsfs_lookup(): missing CMS superblock allocation");
        return NULL;
      }
    if (di->cmssuper->cmsrooti == NULL)
      {
        cmsfs_error(
            (unsigned char *)
            "cmsfs_lookup(): missing CMS dir (root) inode allocation");
        return NULL;
      }

    /*  gimme another CMS inode structure  */
    cmsinode = cmsfs_malloc(sizeof(struct CMSINODE));
    if (cmsinode == NULL)
      {
        cmsfs_error(
            (unsigned char *)
            "cmsfs_lookup(): cannot allocate CMS inode");
        return NULL;
      }

    /*  if we're opening the directory as a file, then ...  */
    if (strncmp(fn,".dir",4) == 0)
      {
#ifdef  CMSFS_DEBUG
        (void) cmsfs_error(
            (unsigned char *)
            "cmsfs_lookup(): opening directory");
#endif
        (void) memcpy(cmsinode,
            di->cmssuper->cmsrooti,sizeof(*cmsinode));
        cmsinode->index = 0;        /*  the number of this inode  */
        di->cmssuper->inuse += 1;   /*  another inode for this super  */
        cmsinode->flags = CMSFSBIN; /*  F 64 so, yes, binary  */
        return cmsinode;            /*  return "success"  */
      }

    /*  set stuff up:  fill-in the structure for this file  */
    cmsinode->cmssuper = di->cmssuper;

    /*  prepare to search:  establish upper-case and exact matches  */
    p = fn;     i = 0;

    /*  first eight bytes (CMS filename)  */
    while ( i < 8 && *p != '.' && *p != 0x00)
      {
#ifdef  CMSFS_HOST_ASCII
        umatch[i] = xmatch[i] = chratoe(*p);
#else
        umatch[i] = xmatch[i] = *p;
#endif
        if (0x80 < umatch[i] && umatch[i] < 0xC0) umatch[i] += 0x40;
        i++;    p++;
      }

    /*  conditionally adjust pointer  */
    if (*p != 0x00) p++;

    /*  possibly pad with blanks  */
    while ( i < 8 )
      {
        umatch[i] = xmatch[i] = 0x40;
        i++;
      }

    /*  second eight bytes (CMS filetype)  */
    while ( i < 16 && *p != '.' && *p != 0x00)
      {
#ifdef  CMSFS_HOST_ASCII
        umatch[i] = xmatch[i] = chratoe(*p);
#else
        umatch[i] = xmatch[i] = *p;
#endif
        if (0x80 < umatch[i] && umatch[i] < 0xC0) umatch[i] += 0x40;
        i++;    p++;
      }

    /*  possibly pad with blanks  */
    while ( i < 16 )
      {
        umatch[i] = xmatch[i] = 0x40;
        i++;
      }

    /*  terminate with NULL character  */
    umatch[i] = xmatch[i] = 0x00;

    /*  find the file  */
    j = di->cmssuper->fstct;
    /*  list the files  */
    for (i = k = 0 ; i < di->cmssuper->files ; i++)
      {
        unsigned
        char *pp;
static  unsigned char buff[4096];
        if (j >= di->cmssuper->fstct)
          {
            (void) cmsfsrd2(di->cmssuper->cmsrooti,buff,k);
            k += 1;     /*  increment block counter  */
            j = 0;      /*  reset intermediate item counter  */
          }
        pp = &buff[j*di->cmssuper->fstsz];
        /*  compare  */
        rc = strncmp(pp,umatch,16);
        if (rc == 0)    /*  found it??  */
          {
            /*  map the inode for the file  */
            rc = cmsfs_map_FST(cmsinode,(struct CMSFSFST *)pp);
            if (rc != 0)
              {
            cmsfs_error(
                    (unsigned char *)
                    "cmsfs_lookup(): cannot map CMS inode (FST)");
                cmsfs_free(cmsinode);
                di->cmssuper->inuse -= 1;
                return NULL;
              }
            cmsinode->index = i;    /*  the number of this inode  */
            di->cmssuper->inuse += 1;   /*  another inode for super  */
            cmsinode->flags = 0x0000;   /*  CALLER MUST APPLY  */
            return cmsinode;            /*  return "success"  */
          }
        j += 1;         /*  increment intermediate item counter  */
      }

    di->cmssuper->error = di->cmssuper->cmsrooti->error = CMSFS_ENOENT;
/*  cmsfs_error(
        (unsigned char *)
        "cmsfs_lookup(): file not found");  */
    (void) sprintf((char*)cmsfs_ermsg,
        "cmsfs_lookup(): file '%s' not found",fn);
    cmsfs_error(cmsfs_ermsg);
    cmsfs_free(cmsinode);
/*  di->cmssuper->inuse -= 1;  */
    return NULL;
  }

/* ---------------------------------------------------------- CMSFS_READ
 *  Read a record at the current pointer in the CMS file.
 *  Simple:  read the record,  increment the record counter.
 */
ssize_t cmsfs_read(struct CMSINODE * cmsfil,void * recbuf,size_t reclen)
  {
    int         i, l, rc;
    unsigned
    char       *bufi, *bufo;

    /*  sanity check:  file pointer must non-NULL  */
    if (cmsfil == NULL)
      { cmsfs_error(
          (unsigned char *)
          "cmsfs_read(): NULL file pointer (inode pointer)");
        return -1; }

    /*  sanity check:  superblock pointer must non-NULL  */
    if (cmsfil->cmssuper == NULL)
      { cmsfs_error(
          (unsigned char *)
          "cmsfs_read(): NULL volume pointer in file struct");
        return -1; }

    /*  can't read past end of file  */
    if (cmsfil->rdpnt >= cmsfil->items)
      {
/*      cmsfil->error = EEOF;                                         *
 *      cmsfs_error("cmsfs_read(): attempted read past end-of-file"); *
 *      return -1;  **  negative implies error                        */
        return 0;  /*  zero clearly means  "no data available"  */
      }

    /*  possibly allocate a work buffer  */
    if (cmsfil->rdbuf == NULL)
      { cmsfil->rdbuf = cmsfs_malloc(cmsfil->cmssuper->blksz);
    if (cmsfil->rdbuf == NULL)
      { cmsfs_error(
          (unsigned char *)
          "cmsfs_read(): unable to allocate a work buffer");
        return -1; }
#ifdef  CMSFS_DEBUG
(void) sprintf((char*)cmsfs_ermsg,
                        "cmsfs_read(): '%s', blk %d, rec %d (#1)",
                        cmsfil->name,cmsfil->rdblk,cmsfil->rdpnt);
cmsfs_error(cmsfs_ermsg);
#endif

    /*  now load-up a block into this new buffer  */
        rc = cmsfsrd2(cmsfil,cmsfil->rdbuf,cmsfil->rdblk);
        if (rc != cmsfil->cmssuper->blksz)
          { cmsfs_error(
              (unsigned char *)
              "cmsfs_read(): could not read block");
            return -1; }
        cmsfil->rdblk += 1;
        cmsfil->rdoff = 0;
      }

    /*  dereference VOID buffers to CHAR buffers  */
    bufi = cmsfil->rdbuf;
    bufo = recbuf;

    /*  how big is this record?  */
    switch (cmsfil->recfm[0])
      {
        case 'F':   l = cmsfil->lrecl;
                    break;
        case 'V':
                if (cmsfil->rdoff >= cmsfil->cmssuper->blksz)
                  {
#ifdef  CMSFS_DEBUG
(void) sprintf((char*)cmsfs_ermsg,
                        "cmsfs_read(): '%s', blk %d, rec %d (#2A)",
                        cmsfil->name,cmsfil->rdblk,cmsfil->rdpnt);
cmsfs_error(cmsfs_ermsg);
#endif
                    rc = cmsfsrd2(cmsfil,cmsfil->rdbuf,cmsfil->rdblk);
                    if (rc != cmsfil->cmssuper->blksz)
                      { cmsfs_error(
                          (unsigned char *)
                          "cmsfs_read(): could not read block");
                        return -1; }
                    cmsfil->rdblk += 1;
                    cmsfil->rdoff = 0;
                  }
                    l = bufi[cmsfil->rdoff++] & 0xFF;
                if (cmsfil->rdoff >= cmsfil->cmssuper->blksz)
                  {
#ifdef  CMSFS_DEBUG
(void) sprintf((char*)cmsfs_ermsg,
                        "cmsfs_read(): '%s', blk %d, rec %d (#2B)",
                        cmsfil->name,cmsfil->rdblk,cmsfil->rdpnt);
cmsfs_error(cmsfs_ermsg);
#endif
                    rc = cmsfsrd2(cmsfil,cmsfil->rdbuf,cmsfil->rdblk);
                    if (rc != cmsfil->cmssuper->blksz)
                      { cmsfs_error(
                          (unsigned char *)
                          "cmsfs_read(): could not read block");
                        return -1; }
                    cmsfil->rdblk += 1;
                    cmsfil->rdoff = 0;
                  }
                    l = (l * 256) + (bufi[cmsfil->rdoff++] & 0xFF);
                    break;
        default:    /*  Bzzzttt!!!  */
                cmsfs_error(
                    (unsigned char *)
                    "cmsfs_read(): RECFM not 'F' or 'V'");
                return -1;
      }

    if (l > reclen)
      {
        (void) sprintf((char*)cmsfs_ermsg,
            "cmsfs_read(): record (%d) is longer than buffer (%d)",
            l,reclen);
        cmsfs_error(cmsfs_ermsg);
/* ADDED FOR DIAGNOSTICS */
        (void) sprintf((char*)cmsfs_ermsg,
            "cmsfs_read(): %s %s LRECL=%d RECFM=%s RECS=%d SIZE=%d",
            cmsfil->fname,cmsfil->ftype,cmsfil->lrecl,cmsfil->recfm,
            cmsfil->items,cmsfil->bytes);
        cmsfs_error(cmsfs_ermsg);
        return -1;
      }

    /*  copy bytes from work buffer to caller's buffer  */
    for (i = 0; i < l; i++)
      {
        if (cmsfil->rdoff >= cmsfil->cmssuper->blksz)
          {
#ifdef  CMSFS_DEBUG
(void) sprintf((char*)cmsfs_ermsg,
                        "cmsfs_read(): '%s' blk %d rec %d (#3)",
                        cmsfil->name,cmsfil->rdblk,cmsfil->rdpnt);
cmsfs_error(cmsfs_ermsg);
#endif
            rc = cmsfsrd2(cmsfil,cmsfil->rdbuf,cmsfil->rdblk);
            if (rc != cmsfil->cmssuper->blksz)
              { cmsfs_error(
                  (unsigned char *)
                  "cmsfs_read(): could not read block");
                return -1; }
            cmsfil->rdblk += 1;
            cmsfil->rdoff = 0;
          }
        bufo[i] = bufi[cmsfil->rdoff++];
      }

    /*  terminate the record (with NULL and possibly also NL)  */
    if (cmsfil->flags & CMSFSTXT)
      {
#ifdef  CMSFS_HOST_ASCII
        bufo[i] = 0x00;
        (void) stretoa(bufo); 
#endif
        bufo[i++] = '\n';
      }
    bufo[i] = 0x00; 

    cmsfil->rdpnt += 1;         /*  increment record pointer  */

    return i;                   /*  return length of string copied  */
  }

/* --------------------------------------------------------- CMSFS_BYTES
 *  Return the size-in-bytes of a CMS file and stamp it in the struct.
 */
long int cmsfs_bytes(struct CMSINODE * fi)
  {
    int         i, j, k, l;

#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,"cmsfs_bytes(0x%08X)",fi);
    cmsfs_error(cmsfs_ermsg);
#endif

    /*  quick return if this function has been called before  */
    if (fi->bytes != 0) return fi->bytes;

    /*  if there are no records,  then there ain't no bytes  */
    if (fi->items == 0) return fi->bytes = 0;

    /*  in case conversion is not already known,  do this  */
    if (fi->flags == 0x0000) cmsfs_map_EXT(fi);

    /*  how big is this file?  "it depends"  */
    switch (fi->recfm[0])
      {
        case 'F':   /*  for fixed-length,  total size is easy  */
                    fi->bytes = fi->lrecl * fi->items;
                    /*  for text files, add NL to each record  */
                    if (fi->flags & CMSFSTXT) fi->bytes += fi->items;
#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,
        "cmsfs_bytes(): computed %d bytes RECFM=F",fi->bytes);
    cmsfs_error(cmsfs_ermsg);
#endif
                    break;

        case 'V':   /*  for variable-length,  we must read content  */
            l = fi->lrecl + 2;
            /*  allocated a work buffer of LRECL size  (plus two)  */
            if (fi->rdbuf2 == NULL) fi->rdbuf2 = cmsfs_malloc(l);
            if (fi->rdbuf2 == NULL)
              {
                cmsfs_error(
            (unsigned char *)
                    "cmsfs_lookup(): cannot allocate CMS inode");
                return -1;
              }
            k = 0;              /*  byte total starts at zero  */
            fi->rdpnt = 0;      /*  record counter starts at zero  */
            /*  read every record, totalling up the bytes  */
            for (i = 0 ; i < fi->items ; i++)
              { j = cmsfs_read(fi,fi->rdbuf2,l);
                if (j < 0) return j;
                k = k +j; }
            fi->bytes = k;
#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,
        "cmsfs_bytes(): counted %d bytes RECFM=V",fi->bytes);
    cmsfs_error(cmsfs_ermsg);
#endif
            break;

        default:    fi->error = CMSFS_EINTERNAL;  /*  internal error  */
                    cmsfs_error(
                    (unsigned char *)
                    "cmsfs_bytes(): RECFM not 'F' or 'V'");
                    return 0;
      }

    return fi->bytes;
  }



