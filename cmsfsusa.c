/*
 *
 *        Name: cmsfsusa.c (C program source)
 *              CMS FS User Space Application routines
 *        Date: 2000-Sep-19 (Tue)
 *
 *    See also: cmsfsany.c (common) and cmsfsvfs.c (driver)
 *
 *              This source contains the routines and structures
 *              required when CMS FS is built as a user-space utility.
 *
 */

#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "cmsfs.h"
/* include "aecs.h" */

/* --------------------------------------------------------- CMSFS_ERROR
 *  Generally a printk() operation, not really perror() function.
 */
void cmsfs_error(unsigned char * string)
  {
    (void) fprintf(stderr,"%s\n",string);
    (void) fflush(stderr);
  }

/* --------------------------------------------------------- CMSFS_BREAD
 *  The "b" here means "block", not "buffer".  Do not confuse this with
 *  Linux VFS bread() function.   This is CMS FS "block read" function.
 *
 *  Block numbers here are offset,  unlike CMS FS internally.
 *  That is,  subtract one from the on-disk pointers before calling.
 */
int cmsfs_bread(struct CMSSUPER *vol,void *buf,int block,int blocksize)
  {
    int         rc;

#ifdef  CMSFS_DEBUG
    (void) sprintf((char*)cmsfs_ermsg,
        "cmsfs_bread(,,%d,%d)",block,blocksize);
    cmsfs_error(cmsfs_ermsg);
#endif

    /*
     *  This is the utility mode cmsfs_bread()
     *  so the vfssuper pointer should be NULL.   Confirm that.
     */
    if (vol->vfssuper != NULL)  /*  are we in "utility mode"?  */
      {
        cmsfs_error(
            (unsigned char *)
            "cmsfs_bread(): non-NULL VFS superblock pointer");
        return -1;
      }

    /*  seek to the block we're after  */
    rc = lseek(vol->devfd,block*blocksize,SEEK_SET);
    if (rc != block*blocksize)
      {
        (void) perror("lseek()");
        (void) sprintf((char*)cmsfs_ermsg,
            "cmsfs_bread(): error from lseek(), RC=%d",rc);
        cmsfs_error((unsigned char *)cmsfs_ermsg);
        return -1;
      }

    /*  read that block  */
    rc = read(vol->devfd,buf,blocksize);
    if (rc != blocksize)
      {
        (void) perror("read()");
        (void) sprintf((char*)cmsfs_ermsg,
            "cmsfs_bread(): error from read(), RC=%d",rc);
        cmsfs_error(cmsfs_ermsg);
        return -1;
      }

    return blocksize;
  }

/* -------------------------------------------------------- CMSFS_MALLOC
 *  Ordinary malloc() call, since we are in user space.
 *  There is a similar wrapper around kmalloc() for the FS driver.
 */
/* include <malloc.h> */
#include <stdlib.h>
void * cmsfs_malloc(int size)
  {
    return (void*) malloc(size);
  }

/* ---------------------------------------------------------- CMSFS_FREE
 *  Ordinary free() call, since we are in user space.
 *  There is a similar wrapper around kfree() for the FS driver.
 */
void cmsfs_free(void * buffer)
  {
    free(buffer);
    return;
  }

/* ------------------------------------------------------------ CMSFSBEX
 *  Convert a big-endian integer into a local integer the slow way.
 *  We do it this way in the utility to make it more reliable.
 *  The driver can leverage kernel macros for optimal performance.
 *  This function moved to this source file from cmsfsany.c for 1.1.8.
 */
int cmsfsbex ( unsigned char *ivalue , int l )
  {
    int         i , ovalue ;
    ovalue = ivalue[0] ;
    for (i = 1 ; i < l ; i++)
      {
        ovalue = ovalue << 8 ;
        ovalue += ivalue[i] & 0xFF;
      }
    return ovalue ;
  }



/* ================================================================== */
/* include the common code here */


#define         cmsfs_mktime(p)         mktime(p)
#define be32_to_cpu(i) cmsfsbex((unsigned char *)&(i),4)
#define be16_to_cpu(i) cmsfsbex((unsigned char *)&(i),2)
#include "cmsfsany.c"


/* utility mode stuff follows ... */
/* ================================================================== */



/* --------------------------------------------------------- CMSFS_VOPEN
 *  Open a volume.   Volume may be an ordinary file.
 */
struct CMSSUPER * cmsfs_vopen(unsigned char * dn)
  {
    struct  CMSSUPER  *cmssuper;
    int     rc;

    /*  utility mode "volume open"  */
#ifdef  CMSFS_DEBUG
    cmsfs_error(
        (unsigned char *)
        "cmsfs_vopen(), aka: cmsfs_mount() from user space");
#endif

    /*  first things first:  gimme a CMS superblock  */
    cmssuper = cmsfs_malloc(sizeof(struct CMSSUPER));
    if (cmssuper == NULL)
      {
        cmsfs_error((unsigned char *)"cannot allocate CMS superblock");
        return NULL;
      }
    cmssuper->vfssuper = NULL;         /*  we are NOT in kernel mode  */
    cmssuper->pbksz = 0;               /*  we are NOT in kernel mode  */

    /*  open the filesystem as an ordinary file  */
    cmssuper->devfd = open((char*)dn,O_RDONLY);
    if (cmssuper->devfd < 0)
      {
        (void) perror("open()");
        (void) sprintf((char*)cmsfs_ermsg,"cannot open '%s'",dn);
        cmsfs_error(cmsfs_ermsg);
        cmsfs_free(cmssuper);
        return NULL;
      }

#ifdef          LINUX
    /*  several attempts to flush buffered content for this volume  */
    (void) ioctl(_cmssuper->devfd,BLKFLSBUF,0x0000);
    (void) ioctl(_cmssuper->devfd,BLKRRPART,0x0000);
    /*  set the device to read-only status  */
    (void) ioctl(_cmssuper->devfd,BLKROSET,1);
#endif          /*  LINUX  */

    /*  we have no root inode ... yet  */
    cmssuper->cmsrooti = NULL;

    /*  validate and map the volume superblock from ADT  */
    rc = cmsfs_map_ADT(cmssuper);
    if (rc != 0)
      {
        (void) sprintf((char*)cmsfs_ermsg,
            "cmsfs_vopen(): cmsfs_map_ADT() returned %d",rc);
        cmsfs_error(cmsfs_ermsg);
        (void) close(cmssuper->devfd);
        if (cmssuper->cmsrooti != NULL) cmsfs_free(cmssuper->cmsrooti);
        cmsfs_free(cmssuper);
        return NULL;
      }

    /*  check your work ... should be a CMS "inode" for root dir  */
    if (cmssuper->cmsrooti == NULL)
      {
        cmsfs_error(
            (unsigned char *)
            "cmsfs_vopen(): cmsfs_map_ADT() set a NULL root inode");
        (void) close(cmssuper->devfd);
        cmsfs_free(cmssuper);
        return NULL;
      }

    /*  we are not in stand-alone mode;  make that painfully clear  */
    cmssuper->devno = 0;
    cmssuper->subch = 0;

    /*  we are not in kernel mode;  make that painfully clear  */
    cmssuper->devid = 0;
    cmssuper->cmsrooti->vfsinode = NULL;

    /*  return "success"  */
    return cmssuper;
  }

/* -------------------------------------------------------- CMSFS_VCLOSE
 *  "Close Function number One" -- close a CMS volume (as Unix file)
 */
int cmsfs_vclose(struct CMSSUPER *vol)
  {
#ifdef  CMSFS_DEBUG
    cmsfs_error((unsigned char *)"cmsfs_vclose()");
#endif

    /*  confirm all files in this filesystem are closed  */
/*  if (vol->inuse != 0)  */
    if (vol->inuse != 1)
      {
        cmsfs_error(
            (unsigned char *)
            "cmsfs_vclose(): cannot close volume with open files");
        vol->error = CMSFS_EBUSY;
        return -1;
      }

    /*  and now close the directory  (free its inode)  */
    if (vol->cmsrooti != NULL)
      {
        (void) cmsfs_fclose(vol->cmsrooti);
        vol->cmsrooti = NULL;
      }

    /*  close the Unix (POSIX) file descriptor  */
    (void) close(vol->devfd);
    vol->devfd = 0;

    /*  zero-out values that serve as checks elsewhere  */
    vol->blksz = 0;
    vol->fstct = 0;
    vol->files = 0;

    /*  now free the CMS superblock structure  */
    cmsfs_free(vol);

    return 0;
  }

/* --------------------------------------------------------- CMSFS_FOPEN
 *  Open a CMS file in an already open CMS volume.
 *  Should be modified to use the cmsfs_lookup() function
 *  from the common code.
 */
struct CMSINODE *
cmsfs_fopen(unsigned char * fn,struct CMSSUPER * cmssuper,int flag)
  {
    int         rc, i, j, k;
    struct  CMSINODE  *cmsinode;
    unsigned char  umatch[18], xmatch[18], *p;
    struct  CMSFSFST  cmsfst;   /*  "FST" structure (per IBM)  */
    int         start;

    /*  utility mode "file open" operation  */
#ifdef  CMSFS_DEBUG
    (void) sprintf((char*)cmsfs_ermsg,"cmsfs_fopen('%s',,)",fn);
    cmsfs_error(cmsfs_ermsg);
#endif

    if (cmssuper == NULL) return NULL;
    if (cmssuper->vfssuper != NULL) return NULL;

    /*  first things first:  check the directory inode structure  */
    if (cmssuper->cmsrooti == NULL)
      {
        cmsfs_error(
            (unsigned char *)
            "cmsfs_fopen(): NULL directory inode pointer");
        return NULL;
      }
    if (cmssuper->cmsrooti->cmssuper != cmssuper)
      {
        cmsfs_error(
            (unsigned char *)
            "cmsfs_fopen(): corrupted directory inode");
        return NULL;
      }

    /*  gimme another CMS inode structure  */
    cmsinode = cmsfs_malloc(sizeof(struct CMSINODE));
    if (cmsinode == NULL)
      {
        cmsfs_error(
            (unsigned char *)
            "cmsfs_fopen(): cannot allocate CMS inode");
        return NULL;
      }

    /*  if we're opening the directory as a file, then ...  */
    if (strncmp(fn,".dir",4) == 0)
      {
#ifdef  CMSFS_DEBUG
        cmsfs_error(
            (unsigned char *)
            "cmsfs_fopen(): opening directory");
#endif
        (void) memcpy(cmsinode,cmssuper->cmsrooti,sizeof(*cmsinode));
        cmssuper->inuse += 1;           /*  incr # files open  */
        cmsinode->flags = flag;         /*  set flags per supplied  */
        return cmsinode;                /*  return "success"  */
      }

    /*  set stuff up:  fill-in the structure for this file  */
    cmsinode->cmssuper = cmssuper;
    cmsinode->vfsinode = NULL;         /*  we are NOT in kernel mode  */

    /*  prepare to search:  establish upper-case and exact match  */
    p = fn;     i = 0;

    /*  first eight bytes (CMS filename)  */
    while ( i < 8 && *p != '.' && *p != 0x00)
      {
#ifdef  CMSFS_HOST_ASCII
        xmatch[i] = chratoe(*p);
        umatch[i] = chratoe(toupper(*p));
#else
        xmatch[i] = *p;
        umatch[i] = toupper(*p);
#endif
        i++;    p++;
      }

    /*  conditionally adjust pointer  */
    if (*p != 0x00) p++;

    /*  possibly pad with blanks  */
    while ( i < 8 )
      {
        xmatch[i] =
        umatch[i] = 0x40;
        i++;
      }

    /*  second eight bytes (CMS filetype)  */
    while ( i < 16 && *p != '.' && *p != 0x00)
      {
#ifdef  CMSFS_HOST_ASCII
        xmatch[i] = chratoe(*p);
        umatch[i] = chratoe(toupper(*p));
#else
        xmatch[i] = *p;
        umatch[i] = toupper(*p);
#endif
        i++;    p++;
      }

    /*  possibly pad with blanks  */
    while ( i < 16 )
      {
        xmatch[i] =
        umatch[i] = 0x40;
        i++;
      }

    /*  terminate with NULL character  */
    xmatch[i] =
    umatch[i] = 0x00;

#ifdef  CMSFS_DEBUG
(void) printf("XMATCH \
%02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X\n",
                xmatch[0],xmatch[1],xmatch[2],xmatch[3],
                xmatch[4],xmatch[5],xmatch[6],xmatch[7],
                xmatch[8],xmatch[9],xmatch[10],xmatch[11],
                xmatch[12],xmatch[13],xmatch[14],xmatch[15]);

(void) printf("UMATCH \
%02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X\n",
                umatch[0],umatch[1],umatch[2],umatch[3],
                umatch[4],umatch[5],umatch[6],umatch[7],
                umatch[8],umatch[9],umatch[10],umatch[11],
                umatch[12],umatch[13],umatch[14],umatch[15]);
#endif

    /*  find the file  */
    j = cmssuper->fstct;
    /*  list the files  */
    for (i = k = 0 ; i < cmssuper->files ; i++)
      {
        unsigned char buff[4096], *pp;
        if (j >= cmssuper->fstct)
          {
            (void) cmsfsrd2(cmssuper->cmsrooti,buff,k);
            k += 1;     /*  increment block counter  */
            j = 0;      /*  reset intermediate item counter  */
          }
        pp = &buff[j*cmssuper->fstsz];
        /*  compare  */
        rc = strncmp(pp,umatch,16);
        if (rc == 0)    /*  found it??  */
          {
            /*  map the inode for the file  */
            rc = cmsfs_map_FST(cmsinode,(struct CMSFSFST *)pp);
            if (rc != 0)
              {
                (void) perror("cmsfs_map_FST()");
                cmsfs_error(
                    (unsigned char *)
                    "cannot map CMS inode (FST)");
                cmsfs_free(cmsinode);
                return NULL;
              }
            cmssuper->inuse += 1;       /*  incr # files open  */
            cmsinode->flags = flag;     /*  set flags per supplied  */
            return cmsinode;            /*  return "success"  */
          }
        j += 1;         /*  increment intermediate item counter  */
      }

    cmssuper->error = cmssuper->cmsrooti->error = CMSFS_ENOENT;
    cmsfs_error((unsigned char *)"file not found");
    return NULL;
  }

/* -------------------------------------------------------- CMSFS_FCLOSE
 *  "Close Function number Two" -- close a CMS file
 */
int cmsfs_fclose(struct CMSINODE *inode)
  {
#ifdef  CMSFS_DEBUG
    cmsfs_error((unsigned char *)"cmsfs_fclose()");
#endif

    if (inode->cmssuper->inuse <= 0)
      {
        cmsfs_error(
            (unsigned char *)
            "cmsfs_fclose(): open files count is corrupted");
        inode->cmssuper->error = EMFILE;
        return -1;
      }

    /*  zero-out values that serve as checks elsewhere  */
    inode->lrecl = 0;
    inode->items = 0;
    inode->bytes = 0;

    inode->cmssuper->inuse -= 1;    /*  decrement number files open
                                    in the volume for this file  */

    /*  now free the structure  */
    if (inode->rdbuf) cmsfs_free(inode->rdbuf);
    if (inode->rdbuf2) cmsfs_free(inode->rdbuf2);
    cmsfs_free(inode);

    return 0;
  }

/* ---------------------------------------------------------- CMSFS_OPEN
 *  Open a CMS file in an as-yet-unopenned CMS volume.
 *  What we do is,  from one path,  glean both CMS volume info
 *  and CMS fileid info.   We open the volume,  then open the file.
 *  This function is useful for one-shot utilities like 'cmsfscp'.
 */
struct CMSINODE * cmsfs_open(unsigned char * filepath,int flag,
                        unsigned char * dn)
  {
    struct  CMSSUPER  *vh;      /*  volume handle  */
    struct  CMSINODE  *fh;      /*  file handle  */
    unsigned  char  *filetemp, *slash, *p, *q, *vn;

    /*  take the default CMS volid,  if it was supplied  */
    vn = (unsigned char*) dn;

    /*  allocate a work buffer for volid & fileid parsing  */
    filetemp = cmsfs_malloc(strlen(filepath)+1);
    if (filetemp == NULL) return NULL;

    /*  copy the string,  looking for slashes  (std path delim)  */
    slash = p = filetemp;
    q = (unsigned char *) filepath;
    while (*q != 0x00)
      {
        *p = *q;
        if (*p == '/') slash = p;
        p++ ; q++ ;
      }

    /*  did we find a slash?  */
    if (*slash == '/')
      {
        /*  divide the path string there into volid and fileid  */
        *slash++ = 0x00;
        /*  and use this volid in the cmsfs_vopen() call below  */
        vn = filetemp;
      }

    /*  are we okay with a CMS FS volume?  */
    if (vn == NULL)
      {
        cmsfs_error(
            (unsigned char *)
            "cmsfs_open(): incomplete path or missing volume spec");
        return NULL;
      }

    /*  ... then let's try to open that volume  */
    vh = cmsfs_vopen(vn);
    if (vh == NULL) return NULL;

    /*  now try to open the file  */
    fh = cmsfs_fopen(slash,vh,flag);
    if (fh == NULL) return NULL;

    return fh;
  }

/* --------------------------------------------------------- CMSFS_CLOSE
 *  Close the CMS file and close the CMS volume too (if possible).
 *  An open CMS file "inode" must point to an open CMS "superblock".
 *  So given an open CMS file,  we also have an open CMS volume.
 *  This function is useful for one-shot utilities like 'cmsfscp'.
 */
int cmsfs_close(struct CMSINODE * cms_file)
  {
    struct  CMSSUPER  *cms_sblk;
    int         rc;

    /*  sanity check:  file structure pointer should not be NULL  */
    if (cms_file == NULL)
      {
        cmsfs_error(
            (unsigned char *)
            "cmsfs_close(): NULL file pointer");
        return -1;
      }

    cms_sblk = cms_file->cmssuper;
    /*  sanity check:  vol structure pointer should not be NULL  */
    if (cms_sblk == NULL)
      {
        cmsfs_error(
            (unsigned char *)
            "cmsfs_close(): NULL volume pointer");
        return -1;
      }

    /*  close the file  */
    rc = cmsfs_fclose(cms_file);
    if (rc != 0) return rc;

    /*  close the volume which holds that file  */
    rc = cmsfs_vclose(cms_sblk);
    if (rc != 0) return rc;

    return 0;
  }

/* ------------------------------------------------------------ CMSFSDMP
 *  Dump buffer contents in hexadecimal.   (just a testing hack)
 */
void cmsfs_xdump(unsigned char *p,int l)
  {
    int              i;
    unsigned  char  *q;

    i = 0;
    while (i < l)
      {
        q = cmsfs_ermsg;

        (void) sprintf(
                (char*)q,"%02X%02X%02X%02X %02X%02X%02X%02X ",
                p[i]&0xFF,p[i+1]&0xFF,p[i+2]&0xFF,p[i+3]&0xFF,
                p[i+4]&0xFF,p[i+5]&0xFF,p[i+6]&0xFF,p[i+7]&0xFF);
        i += 8;  q += strlen(cmsfs_ermsg);
        (void) sprintf(
                (char*)q,"%02X%02X%02X%02X %02X%02X%02X%02X  ",
                p[i]&0xFF,p[i+1]&0xFF,p[i+2]&0xFF,p[i+3]&0xFF,
                p[i+4]&0xFF,p[i+5]&0xFF,p[i+6]&0xFF,p[i+7]&0xFF);
        i += 8;  q += strlen(cmsfs_ermsg);

        (void) sprintf(
                (char*)q,"%02X%02X%02X%02X %02X%02X%02X%02X ",
                p[i]&0xFF,p[i+1]&0xFF,p[i+2]&0xFF,p[i+3]&0xFF,
                p[i+4]&0xFF,p[i+5]&0xFF,p[i+6]&0xFF,p[i+7]&0xFF);
        i += 8;  q += strlen(cmsfs_ermsg);
        (void) sprintf(
                (char*)q,"%02X%02X%02X%02X %02X%02X%02X%02X",
                p[i]&0xFF,p[i+1]&0xFF,p[i+2]&0xFF,p[i+3]&0xFF,
                p[i+4]&0xFF,p[i+5]&0xFF,p[i+6]&0xFF,p[i+7]&0xFF);
        i += 8;  q += strlen(cmsfs_ermsg);

        cmsfs_error(cmsfs_ermsg);
      }
  }


