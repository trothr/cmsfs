/*
 *
 *        Name: cmsfsvfs.c (C program source)
 *              CMS FS VFS interface routines (Linux fs driver code)
 *        Date: 2000-Sep-14 (Thu)
 *
 *              THIS IS FOR THE 2.2 KERNEL
 *
 *    See also: cmsfsany.c (common) and cmsfsusa.c (utility)
 *
 *              This source contains the routines and structures
 *              required when CMS FS is built as a driver.
 *              It interfaces between CMS FS' own structures,
 *              which read the CMS EDF format,  and Linux VFS.
 *
 *  Objectives: Mimic ISO, which is also read-only media
 *              Mimic MS-DOS, which has similar short names (8x3)
 *              and may require content conversion.
 *
 */

#define         __KERNEL__

#include        <linux/module.h>
#include        <linux/version.h>
#include        <linux/fs.h>
#include        <linux/slab.h>
#include        <linux/mm.h>

/*  The following gives us  lock_super()  and  unlock_super().  */
#include        <linux/locks.h>

#include        <linux/init.h>

/*  The following gives us "user space" access,  like  copy_to_user().  */
#include        <asm/uaccess.h>

#define         CMSFS_NEED_TM
#include        "cmsfs.h"

/* --------------------------------------------------------- CMSFS_ERROR
 *  Generally a printk() operation, not really perror() function.
 */
void cmsfs_error(unsigned char * string)
  {
    (void) printk("CMSFS: %s\n",string);
    return;
  }

/* --------------------------------------------------------- CMSFS_BREAD
 *  The "b" here means "block", not "buffer".  Do not confuse this with
 *  Linux VFS bread() function.   This is CMS FS "block read" function.
 */
int cmsfs_bread(struct CMSSUPER *vol,void *buf,int block,int blocksize)
  {
    struct buffer_head * bh;
    struct super_block * sb;

#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,"cmsfs_bread(0x%08X,0x%08X,%d,%d)",
                vol,buf,block,blocksize);
    cmsfs_error(cmsfs_ermsg);
#endif

    /*  for the moment,  we only deal with physical blocks  */
    if (blocksize != vol->pbksz)
      {
        (void) sprintf(cmsfs_ermsg,
"cmsfs_bread(): logical bksize %d does not match physical bksize %d",
                blocksize,vol->pbksz);
        cmsfs_error(cmsfs_ermsg);
        return -1;
      }
    /*  We could maybe handle that case by breaking-up this call into
        multiple bread() calls, but that'll be a later driver rev.  */

/*  ratio = vol->pbksz / blocksize;  */
    sb = (struct super_block *) vol->vfssuper;

    /*  announce and call the system-level bread()  */
#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,"cmsfs_bread(): system bread('%s',%d,%d)",
                kdevname(sb->s_dev),block,blocksize);
    cmsfs_error(cmsfs_ermsg);
#endif
    bh = bread(sb->s_dev,block,blocksize);
    if (bh == NULL)
      {
        cmsfs_error("cmsfs_bread(): system bread() failed");
        return -1;
      }

    /*  copy the data part,  then release the VFS buffer  */
/*  (void) memcpy(buf,bh->b_data,blocksize);  */
    (void) memmove(buf,bh->b_data,blocksize);

    (void) brelse(bh);

/*
    bh = bread(sb->s_dev,block/ratio,vol->pbksz);
    if (bh == NULL) return -1;
 */

    return blocksize;
  }

/* -------------------------------------------------------- CMSFS_MALLOC
 *  Wrapper around kmalloc() for the driver.
 *  There is a similar wrapper around user-mode malloc() for the util.
 */
void * cmsfs_malloc(int size)
  {
    return (void *) kmalloc(size, GFP_KERNEL);
  }

/* ---------------------------------------------------------- CMSFS_FREE
 *  Wrapper around kfree() for the driver.
 *  There is a similar wrapper around user-mode free() for the util.
 */
void cmsfs_free(void * buffer)
  {
    kfree(buffer);
    return;
  }

/* -------------------------------------------------------- CMSFS_MKTIME
 *  It would be much MUCH better to use the mktime() function
 *  found in GLIBC,  but I had trouble compiling in a way that would
 *  let me use it with kernel code.  If someone knows how to use
 *  GLIBC's version of mktime() with kernel code,  PLEASE TELL ME.
 */
time_t cmsfs_mktime(struct cmsfs_tm * source)
  {
    time_t  result;
    int     year, mon, yoff;

    /*  start with nothing  */
    result = 0;

    /*  add days for each year  */
    yoff = source->tm_year + 1900;
    for (year = 1970 ; year < yoff ; year++)
      {
        if      ((year % 400) == 0) result += 366;
        else if ((year % 100) == 0) result += 365;
        else if ((year % 4)   == 0) result += 366;
        else                        result += 365;
      }

    /*  add days for each month  */
    switch (source->tm_mon)
      {
        case  12:   result += 31;       /*  never executed  */
        case  11:   result += 30;
        case  10:   result += 31;
        case  9:    result += 30;
        case  8:    result += 31;
        case  7:    result += 31;
        case  6:    result += 30;
        case  5:    result += 31;
        case  4:    result += 30;
        case  3:    result += 31;
        case  2:    /*  how many days in Feb for this year?  */
                    if      ((source->tm_year % 400) == 0) result += 29;
                    else if ((source->tm_year % 100) == 0) result += 28;
                    else if ((source->tm_year % 4)   == 0) result += 29;
                    else                                   result += 28;
        case  1:    result += 31;
      }

    /*  add days for this month  */
    result += source->tm_mday - 1;

    /*  make days into hours,  then add hours  */
    result = (result * 24) + source->tm_hour;

    /*  make hours into minutes,  then add minutes  */
    result = (result * 60) + source->tm_min;

    /*  make minutes into seconds,  then add seconds  */
    result = (result * 60) + source->tm_sec;

    return result;
  }



/* include the common code here */

/*  but this is defined in /usr/include/asm/string.h  */
/*  #define  memcpy(b,d,l)  memmove(b,d,l)  */

#define  isupper(c)  (c >= 'A' && c <= 'Z')
#define  tolower(c)  (c + ('a' - 'A'))

#include        "cmsfsany.c"
#ifdef  CMSFS_HOST_ASCII
#include        "aecs.c"
#endif

/* filesystem driver stuff follows ... */



/*  I learned of this requirement from Neale's "cpint" code.  */
char kernel_version [] = UTS_RELEASE;

static  unsigned  char  cmsfsflb[4096];

/* ---------------------------------------------------- CMSFS_FILE_LSEEK
 *  CMS FS "lseek" function for VFS file.  NEED TO IMPLEMENT.
 */
loff_t cmsfs_file_lseek(struct file * file, loff_t offset, int whence)
  {
    cmsfs_error("cmsfs_file_lseek() -- N/A");
    return -1;
  }

/* ----------------------------------------------------- CMSFS_FILE_READ
 *  CMS FS "read" function for VFS file.
 *  We prefer to work with inodes,  so extract the inode
 *  from the file pointer,  formally  file->f_dentry->d_inode.
 */
ssize_t cmsfs_file_read(struct file * file,
                char * buffer, size_t length, loff_t * offset)
  {
    struct CMSINODE * ci;
    int     rc;

/*  what's happening here with f_pos??  */
/*
      { int q; q = file->f_pos; (void) sprintf(cmsfs_ermsg,
        "cmsfs_file_read(): VFS ->f_pos %lu",q); }
    cmsfs_error(cmsfs_ermsg);
      { int q; q = *offset; (void) sprintf(cmsfs_ermsg,
        "cmsfs_file_read(): offset %lu",q); }
    cmsfs_error(cmsfs_ermsg);
 */

#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,"cmsfs_file_read(,,%lu,)",length);
    cmsfs_error(cmsfs_ermsg);
#endif

    /*  dereference CMS inode pointer  */
    ci = file->f_dentry->d_inode->u.generic_ip;
    if (ci->vfsinode != file->f_dentry->d_inode)
      {
        (void) sprintf(cmsfs_ermsg,
            "cmsfs_file_read(): CMS struct for inode %lu is corrupted",
            file->f_dentry->d_inode->i_ino);
        cmsfs_error(cmsfs_ermsg);
        return -1;
      }

#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,
        "cmsfs_file_read(): inode %lu, file '%s'",
         file->f_dentry->d_inode->i_ino,ci->name);
    cmsfs_error(cmsfs_ermsg);
#endif

    /*  if translation hasn't been determined,  figure it out  */
    cmsfs_map_EXT(ci);
    /*  default to text files for the time being  */
    if (ci->flags == 0x0000) ci->flags = CMSFSTXT;

    /*  allocate a record buffer,  if needed  */
    if (ci->rdbuf2 == NULL)
        ci->rdbuf2 = cmsfs_malloc(ci->lrecl+2);
    if (ci->rdbuf2 == NULL)
        ci->rdbuf2 = cmsfs_malloc(ci->lrecl+2);
    if (ci->rdbuf2 == NULL)
      { cmsfs_error("cmsfs_file_read(): cannot allocate record buffer");
        return -1; }

    /*  try reading the file  */
    rc = cmsfs_read(ci,ci->rdbuf2,length);
#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,
        "cmsfs_file_read(): cmsfs_read() returned %d for %d",rc,length);
    cmsfs_error(cmsfs_ermsg);
#endif
    if (rc < 0) return rc;

    /*  copy from kernel storage to user storage  */
    if (rc > 0) (void) copy_to_user(buffer,ci->rdbuf2,rc);

    /*  update file pointers  */
    *offset = *offset + rc;
    /*  one or the other,  but not both!  */
/*  file->f_pos += rc;  */

/*
      { int q; q = file->f_pos; (void) sprintf(cmsfs_ermsg,
        "cmsfs_file_read(): VFS ->f_pos %lu",q); }
    cmsfs_error(cmsfs_ermsg);
      { int q; q = *offset; (void) sprintf(cmsfs_ermsg,
        "cmsfs_file_read(): offset %lu",q); }
    cmsfs_error(cmsfs_ermsg);
 */

    return rc;      /*  report how many bytes were read  */
  }

/* -------------------------------------------------- CMSFS_FILE_READDIR
 *  CMS FS "readdir" function for VFS directory.
 *  It took me a long time to figure out to code the right datatype
 *  for the fourth argument to filldir().   [sigh]
 */
int cmsfs_file_readdir(struct file * file,
                                void * dirent, filldir_t myfd)
  {
    struct  CMSINODE  *dirinode, *tmpinode;
    int         rc, i, j, k;

/*
      { int q; q = file->f_pos; (void) sprintf(cmsfs_ermsg,
        "cmsfs_file_readdir(): VFS ->f_pos %lu",q); }
    cmsfs_error(cmsfs_ermsg);
 */

    if (file->f_pos != 0) return 0;

#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,
        "cmsfs_file_readdir(0x%08X,0x%08X,0x%08X)",file,dirent,myfd);
    cmsfs_error(cmsfs_ermsg);
#endif

    tmpinode = cmsfs_malloc(sizeof(struct CMSINODE));
    /*  we should check for errors here!  */

    dirinode = file->f_dentry->d_inode->u.generic_ip;
#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,
        "cmsfs_file_readdir(): inode %lu %d files",
        file->f_dentry->d_inode->i_ino,dirinode->items);
    cmsfs_error(cmsfs_ermsg);
#endif

    dirinode->rdpnt = 0;
    file->f_pos = dirinode->rdpnt * dirinode->lrecl;

    rc = myfd(dirent,".",1,file->f_pos,0);
/*  (void) sprintf(cmsfs_ermsg,
        "cmsfs_file_readdir(): filldir(,'.',,,0) returned %d",rc);
    cmsfs_error(cmsfs_ermsg);                                         */

    dirinode->rdpnt += 1;
    file->f_pos = dirinode->rdpnt * dirinode->lrecl;

    rc = myfd(dirent,"..",2,file->f_pos,
        file->f_dentry->d_parent->d_inode->i_ino);
/*  (void) sprintf(cmsfs_ermsg,
        "cmsfs_file_readdir(): filldir(,'..',,,1) returned %d",rc);
    cmsfs_error(cmsfs_ermsg);
    (void) sprintf(cmsfs_ermsg,
        "                      filldir(,'..',,,%d) returned %d",
        file->f_dentry->d_parent->d_inode->i_ino,rc);
    cmsfs_error(cmsfs_ermsg);                                         */

    dirinode->rdpnt += 1;
    file->f_pos = dirinode->rdpnt * dirinode->lrecl;

    /*  prime the loop with the first block of the directory  */
    /*  starting at 128 gives the effect of skipping two FSTs  */
    j = 128;  k = 0;
    rc = cmsfsrd2(dirinode,cmsfsflb,k);

    /*  skipping .DIRECTOR and .ALLOCMAP,  fill VFS dir with files  */
    for (i = 2 ; i < dirinode->items ; i++)
      {
        (void) cmsfs_map_FST(tmpinode,
                        (struct CMSFSFST *)&cmsfsflb[j]);
        rc = myfd(dirent,tmpinode->name,
            strlen(tmpinode->name),file->f_pos,i);
/*      (void) sprintf(cmsfs_ermsg,
            "cmsfs_file_readdir(): filldir(,'%s',,%d,%d) returned %d",
            tmpinode->name,i*64,i,rc);
        cmsfs_error(cmsfs_ermsg);                                     */

        /*  if the filldir() call failed,  get outta here  */
        if (rc != 0) break;

        j += 64;

        if (j >= dirinode->cmssuper->blksz)
          {
            k++;
            rc = cmsfsrd2(dirinode,cmsfsflb,k);
            j = 0;
          }

        dirinode->rdpnt += 1;
        file->f_pos = dirinode->rdpnt * dirinode->lrecl;
      }

#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,
        "cmsfs_file_readdir(): CMS rec pos %d",dirinode->rdpnt);
    cmsfs_error(cmsfs_ermsg);
/*  file->f_pos = dirinode->rdpnt * dirinode->lrecl;                  */
      { int q; q = file->f_pos; (void) sprintf(cmsfs_ermsg,
        "cmsfs_file_readdir(): VFS ->f_pos %lu",q); }
    cmsfs_error(cmsfs_ermsg);

    (void) sprintf(cmsfs_ermsg,
        "cmsfs_file_readdir(): stored %d",i);
    cmsfs_error(cmsfs_ermsg);
#endif

    cmsfs_free(tmpinode);
/*  cmsfs_error("cmsfs_file_readdir(): finis!");  */

/*  return i;  */
    return 0;
  }

/* ----------------------------------------------------- CMSFS_FILE_OPEN
 *  CMS FS "open" function for VFS file.
 *  This is just a check in this implementation.
 */
ssize_t cmsfs_file_open(struct inode * in, struct file * file)
  {
    struct CMSINODE * ci;

#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,
        "cmsfs_file_open(0x%08X,0x%08X)",in,file);
    cmsfs_error(cmsfs_ermsg);
#endif

    /*  dereference CMS inode pointer and check cross-link  */
    ci = in->u.generic_ip;
    if (ci == NULL)
      {
        (void) sprintf(cmsfs_ermsg,
            "cmsfs_open(): NULL CMS struct pointer for inode %lu",
            in->i_ino);
        cmsfs_error(cmsfs_ermsg);
      }
    if (ci->vfsinode != in)
      {
        (void) sprintf(cmsfs_ermsg,
            "cmsfs_open(): corrupted CMS struct for inode %lu",
            in->i_ino);
        cmsfs_error(cmsfs_ermsg);
        return -1;
      }

#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,
        "cmsfs_file_open(): opening file '%s' inode %lu",
        ci->name,in->i_ino);
    cmsfs_error(cmsfs_ermsg);
#endif

    /*  reset CMS and VFS pointers  */
    ci->rdpnt = ci->rdblk = ci->rdoff = 0;
    file->f_pos = 0;

    return 0;
  }

/* ------------------------------------------------------------------ */
static
struct file_operations cmsfs_file_operations = {
        cmsfs_file_lseek,
        cmsfs_file_read,
        NULL,   /* write */
        cmsfs_file_readdir,
        NULL,   /* poll */
        NULL,   /* ioctl */
        NULL,   /* mmap */
        cmsfs_file_open,
        NULL,   /* flush */
        NULL,   /* release */
        NULL,   /* fsync */
        NULL,   /* fasync */
        NULL,   /* check_media_change */
        NULL,   /* revalidate */
        NULL    /* lock */
};

/* -------------------------------------------------- CMSFS_INODE_LOOKUP
 *  Search for the file in the CMS directory.
 *  Calls:  cmsfs_lookup() to find the file,
 *          iget() to get a VFS inode for the file, if found,
 *          d_add() to add that inode to the VFS directory.
 */
struct dentry *cmsfs_inode_lookup(
        struct inode * in, struct dentry *de)
  {
    struct CMSINODE * cmsinode;
    struct inode * inode;
    int         n;

#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,"cmsfs_inode_lookup(0x%08X,0x%08X)",in,de);
    cmsfs_error(cmsfs_ermsg);
    (void) sprintf(cmsfs_ermsg,
                "cmsfs_inode_lookup(): dir inode, %lu filename '%s'",
                        de->d_inode,de->d_name.name);
    cmsfs_error(cmsfs_ermsg);
#endif

    /*
     *  On CMS minidisks,  the directory entries (FST) serve as inodes.
     *  Here we allocate one in doing the search,  and then free it.
     *  Not elegant,  but works,  and interfaces properly with VFS.
     */
    cmsinode = cmsfs_lookup((CMSINODE*) in->u.generic_ip,
                (unsigned char *) de->d_name.name);
    if (cmsinode == NULL) return NULL;

    /*  it would be nice to be able to hand-off the CMS inode struct  *
     *  rather than going through another allocation from iget().     */
    n = cmsinode->index;
    cmsinode->cmssuper->inuse -= 1;
    cmsfs_free(cmsinode);

#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,
                "cmsfs_inode_lookup(): found inode %d",n);
    cmsfs_error(cmsfs_ermsg);
#endif

    /*  iget() will allocate a VFS inode.         *
     *  it will also call cmsfs_read_inode(),     *
     *  thus re-allocating the CMS inode struct.  */
    inode = iget(in->i_sb,n);
    if (inode == NULL) return NULL;

#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,
            "cmsfs_inode_lookup(): iget() supplied inode 0x%08X",inode);
    cmsfs_error(cmsfs_ermsg);
#endif

    d_add(de,inode);

/*  cmsfs_error("cmsfs_inode_lookup(): finis!");  */
    return NULL;
  }

/* ---------------------------------------------- CMSFS_INODE_PERMISSION
 *  CMS FS "permission" function for VFS inode.
 *  This is really only useful for filesystems with ACL support.
 *  Return non-zero if you want access to be denied to this inode.
 */
int cmsfs_inode_permission(struct inode * in, int mask)
  {
#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,
        "cmsfs_inode_permission(0x%08X,0%05o)",in,mask);
    cmsfs_error(cmsfs_ermsg);
    (void) sprintf(cmsfs_ermsg,
        "cmsfs_inode_permission(): inode %lu mode 0%05o",
                in->i_ino,in->i_mode);
    cmsfs_error(cmsfs_ermsg);
#endif
    return 0;
  }

/* ------------------------------------------------------------------ */
static
struct inode_operations cmsfs_inode_operations = {
        &cmsfs_file_operations,
        NULL,   /* create */
        cmsfs_inode_lookup,
        NULL,   /* link */
        NULL,   /* unlink */
        NULL,   /* symlink */
        NULL,   /* mkdir */
        NULL,   /* rmdir */
        NULL,   /* mknod */
        NULL,   /* rename */
        NULL,   /* readlink */
        NULL,   /* follow_link */
        NULL,   /* readpage */
        NULL,   /* writepage */
        NULL,   /* bmap */
        NULL,   /* truncate */
        NULL,   /* permission */
        NULL,   /* smap */
        NULL,   /* updatepage */
        NULL    /* revalidate */
};

/* ---------------------------------------------------- CMSFS_READ_INODE
 *  CMS FS inode numbers are base-zero index into the CMS directory.
 *  CMS FS (EDF) is flat, no sub-directories, so there is only
 *  one such directory into which we index, always the "root".
 *  If the inode we are reading is zero,  we mark it as the directory.
 *  The CMS superblock struct must have already been allocated,
 *  although CMS inode structs will usually be allocated here.
 */
void cmsfs_read_inode(struct inode * in)
  {
    struct  CMSINODE  *cmsinode;
    struct  CMSSUPER  *cmssuper;
    int     rc, ct;

#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,"cmsfs_read_inode(0x%08X)",in);
    cmsfs_error(cmsfs_ermsg);
    (void) sprintf(cmsfs_ermsg,
        "cmsfs_read_inode(): reading inode %lu from dev '%s' sb 0x%08X",
                        in->i_ino,kdevname(in->i_dev),in->i_sb);
    cmsfs_error(cmsfs_ermsg);
#endif

    /*  dummy values in case any of this fails  */
    in->i_uid = 0;  in->i_gid = 0;
    in->i_mode = 0;  in->i_nlink = 0;  in->i_size = 0;
    in->i_atime = in->i_mtime = in->i_ctime = 0;
    in->i_blksize = 0;

    /*  dereference pointer to CMS superblock from VFS superblock  */
    cmssuper = (CMSSUPER *) in->i_sb->u.generic_sbp;
    /*  and check it ...  */
    if (cmssuper == NULL)
      { cmsfs_error("cmsfs_read_inode(): no CMS superblock");
        return; }
    if (cmssuper->vfssuper != in->i_sb)
      { cmsfs_error("cmsfs_read_inode(): corrupted CMS superblock");
        return; }

    /*  allocate a CMS inode struct  */
    if (in->u.generic_ip == NULL)
      {
        if (in->i_ino == 0 && cmssuper->cmsrooti != NULL)
          {
#ifdef  CMSFS_DEBUG
            cmsfs_error(
        "cmsfs_read_inode(): using CMS directory inode (prev alloc)");
#endif
            cmsinode = cmssuper->cmsrooti;
            /*  and check it  */
            if (cmsinode->cmssuper != cmssuper) { cmsfs_error(
                "cmsfs_read_inode(): corrupted CMS directory inode");
                return; }
            ct = 0;
          }
        else
          {
#ifdef  CMSFS_DEBUG
            cmsfs_error(
                "cmsfs_read_inode(): allocating a CMS inode struct");
#endif
            cmsinode = cmsfs_malloc(sizeof(struct CMSINODE));
            if (cmsinode != NULL) cmsinode->cmssuper = NULL;
            ct = 1;
          }
      }
    if (cmsinode == NULL)
      {
        cmsfs_error(
            "cmsfs_read_inode(): error allocating CMS inode structure");
        return;
      }

    /*  cross-link CMS and VFS inode structures  */
    cmsinode->vfsinode = in;
    in->u.generic_ip = cmsinode;
#ifdef  CMSFS_DEBUG
    cmsfs_error(
        "cmsfs_read_inode(): VFS and CMS inodes allocated and linked");
#endif

    /*  Non-NULL CMS superblock pointer implies that we have          *
     *  already mapped the CMS inode.   A CMS superblock is           *
     *  required for the cmsfs_map_FST() call to work.                */
    if (cmsinode->cmssuper == NULL)
      {
        cmsinode->cmssuper = cmssuper;

    /*  read the CMS "inode" (FST entry) indexed by in->i_ino ,       *
     *  map it to the newly allocated CMS inode struct,               *
     *  and copy relevent content to the supplied VFS inode struct    */
/*  RECHECK THIS LOGIC for block calc against different blocksizes  */
/*
(void) sprintf(cmsfs_ermsg,"... reading block %d",
                ((in->i_ino)*64)/(in->i_sb->s_blocksize));
cmsfs_error(cmsfs_ermsg);
 */
    rc = cmsfsrd2(cmssuper->cmsrooti,cmsfsflb,
                ((in->i_ino)*64)/(in->i_sb->s_blocksize));
/*
(void) sprintf(cmsfs_ermsg,"cmsfs_read_inode(): cmsfsrd2() returned %d",rc);
cmsfs_error(cmsfs_ermsg);

(void) sprintf(cmsfs_ermsg,"... FST offset %d",
        (in->i_ino%(in->i_sb->s_blocksize/64))*64);
cmsfs_error(cmsfs_ermsg);
 */
    (void) cmsfs_map_FST(cmsinode,(struct CMSFSFST *)
        &cmsfsflb[(in->i_ino%(in->i_sb->s_blocksize/64))*64]);
      }

    /*  create VFS inode from CMS inode  */
    in->i_uid = 0;  in->i_gid = 0;
    in->i_mode = S_IRUGO;               /* set "r--r--r--" perms */
    in->i_nlink = 1;

    /*  how big is this file?  */
    in->i_size = cmsfs_bytes(cmsinode);

    /*  CMS files have only one time stamp  */
    in->i_atime = in->i_mtime = in->i_ctime = cmsinode->ctime;

    /*  CMS file blocksize is by definition filesystem blocksize  */
    in->i_blksize = cmsinode->cmssuper->blksz;

    /*  may not be accurate for total  */
    in->i_blocks = cmsinode->bloks;

    in->i_attr_flags = 0;
    in->i_state = 0;
    in->i_flags = 0;
    in->i_generation = 0;
    in->i_writecount = 0;

    /*  now point to our custom inode_operations struct  */
    in->i_op = &cmsfs_inode_operations;
    /*  CMS FS uses the same i_op struct for files and the directory  */

    /*  is this a directory??  */
    if (in->i_ino == 0)
      {
        in->i_mode |= S_IFDIR;          /* mark it as a directory */
        /*  CMS directory was created when the filesystem was made  */
        in->i_ctime = cmssuper->ctime;
#ifdef  CMSFS_DEBUG
        cmsfs_error(
           "cmsfs_read_inode(): inode zero (root) marked as directory");
#endif
        in->i_nlink += 1;
        in->i_mode |= S_IXUGO;          /* add "--x--x--x" perms */
      }
    /*  we know that inode zero is the CMS directory  */
    else in->i_mode |= S_IFREG;         /* mark it as a regular file */

#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,
        "cmsfs_read_inode(): CMS superblock usage was %d",
        cmssuper->inuse);
    cmsfs_error(cmsfs_ermsg);
#endif

    /*  increment the "in use" counter  */
    cmssuper->inuse += ct;

#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,
        "cmsfs_read_inode(): CMS superblock usage now %d",
        cmssuper->inuse);
    cmsfs_error(cmsfs_ermsg);
#endif

    return;
  }

/* --------------------------------------------------- CMSFS_CLEAR_SUPER
 *  Clears the cross-links and frees the CMS superblock struct.
 *  Cannot lock/unlock the superblock at this stage of the game.
 */
void cmsfs_clear_super(struct super_block * sb)
  {
    CMSSUPER  *cmssuper;

#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,"cmsfs_clear_super(0x%08X)",sb);
    cmsfs_error(cmsfs_ermsg);
#endif

    /*  dereference the CMS superblock pointer and check  */
    cmssuper = sb->u.generic_sbp;
    /*  if we've already been here,  then silently return  */
    if (cmssuper == NULL) return;

    /*  sanity check,  cross-reference CMS and VFS superblocks  */
    if (cmssuper->vfssuper != sb)
      {
        (void) sprintf(cmsfs_ermsg,
           "cmsfs_clear_super(): CMS superblock at 0x%08X is corrupted",
            cmssuper);
        cmsfs_error(cmsfs_ermsg);
        return;
      }

    /*  if superblock is still busy,  then bail  */
    if (cmssuper->inuse != 0)
      {
        (void) sprintf(cmsfs_ermsg,
            "cmsfs_clear_super(): CMS superblock at 0x%08X is busy",
            cmssuper);
        cmsfs_error(cmsfs_ermsg);
        return;
      }

    /*  free the CMSSUPER struct and mark it freed  */
/*  lock_super(sb);  */
    cmssuper->vfssuper = NULL;
    cmsfs_free(cmssuper);       /*  deref'd from sb->u.generic_sbp  */
    sb->u.generic_sbp = NULL;
/*  unlock_super(sb);  */

    /*  one less instance of this driver  */
    MOD_DEC_USE_COUNT;
    /*  don't do this until all storage is freed  */

    return;
  }

/* --------------------------------------------------- CMSFS_CLEAR_INODE
 *  Cleanly deallocate the CMS inode linked to this VFS inode.
 *  Because of the order in which VFS objects are released at umount(),
 *  this function must clear the CMS superblock
 *  if the inode being cleared is the directory.
 */
void cmsfs_clear_inode(struct inode * in)
  {
    struct CMSINODE * ck;
    struct CMSSUPER * cs;
    struct super_block * sb;

#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,"cmsfs_clear_inode(0x%08X)",in);
    cmsfs_error(cmsfs_ermsg);
    (void) sprintf(cmsfs_ermsg,
        "cmsfs_clear_inode(): clearing inode number %lu",in->i_ino);
    cmsfs_error(cmsfs_ermsg);
#endif

    /*  dereference the generic pointer to a CMS struct  */
    ck = in->u.generic_ip;

    /*  if no CMS inode to free,  then silently return  */
    if (ck == NULL) return;

    /*  if ->vfsinode doesn't point back to VFS inode,  then error  */
    if (ck->vfsinode != in)
      {
        cmsfs_error("cmsfs_clear_inode(): error in CMS inode ref");
        (void) sprintf(cmsfs_ermsg,
            "cmsfs_clear_inode(): CMS links to 0x%08X",ck->vfsinode);
        cmsfs_error(cmsfs_ermsg);
        return;
      }

    /*  dereference VFS and CMS superblock pointers for shorthand  */
    sb = in->i_sb;      cs = ck->cmssuper;

    /*  sanity check those,  expecting them to be cross-linked  */
    if (sb->u.generic_sbp != cs)
      { cmsfs_error(
            "cmsfs_clear_inode(): VFS SB ref to CMS SB is corrupted");
        return; }
    if (cs->vfssuper != sb)
      { cmsfs_error(
            "cmsfs_clear_inode(): CMS SB ref to VFS SB is corrupted");
        return; }

#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,
        "cmsfs_clear_inode(): CMS superblock usage was %d",cs->inuse);
    cmsfs_error(cmsfs_ermsg);
#endif

    /*  decrement the "in use" counter  */
    cs->inuse -= 1;

#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,
        "cmsfs_clear_inode(): CMS superblock usage now %d",cs->inuse);
    cmsfs_error(cmsfs_ermsg);
#endif

    if (ck->rdbuf != NULL)
      {
#ifdef  CMSFS_DEBUG
        cmsfs_error("cmsfs_clear_inode(): freeing read buffer");
#endif
        cmsfs_free(ck->rdbuf);
        ck->rdbuf = NULL;
        ck->rdoff = 0;
      }

    if (ck->rdbuf2 != NULL)
      {
#ifdef  CMSFS_DEBUG
        cmsfs_error("cmsfs_clear_inode(): freeing span buffer");
#endif
        cmsfs_free(ck->rdbuf2);
        ck->rdbuf2 = NULL;
        ck->rdoff2 = 0;
      }

#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,
        "cmsfs_clear_inode(): freeing CMS inode at 0x%08X",ck);
    cmsfs_error(cmsfs_ermsg);
#endif

    /*  reset some values to clean-up the storage  */
    ck->vfsinode = NULL;
    ck->cmssuper = NULL;
    /*  now free the storage  */
    cmsfs_free(ck);
    /*  and clear the VFS ref to CMS  */
    in->u.generic_ip = NULL;

    /*  was that the root (CMS dir) inode?  */
    if (ck != cs->cmsrooti) return;
    /*  if it wasn't root,  then we're done  */

    /*  should be safe to free the CMS superblock  (ref'd by VFS)  */
    cmsfs_clear_super(sb);

    return;
  }

/* -------------------------------------------------------- CMSFS_UMOUNT
 *  This is called technically to "begin umount".
 *  Ordinarily a no-op because CMS FS is read-only.
 *  Cannot be used as the code path to free the superblock
 *  because of the order in which VFS comes down when unmounting.

                CLEAN THIS UP!!!      Remove lock() unlock()

 */
void cmsfs_umount(struct super_block * sb)
  {
    CMSSUPER  *cmssuper;

#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,"cmsfs_umount(0x%08X)",sb);
    cmsfs_error(cmsfs_ermsg);
#endif

    /*  dereference the CMS superblock pointer and check  */
    cmssuper = sb->u.generic_sbp;
    /*  if we've already been here,  then silently return  */
    if (cmssuper == NULL) return;

    /*  sanity check,  cross-reference CMS and VFS superblocks  */
    if (cmssuper->vfssuper != sb)
      {
        (void) sprintf(cmsfs_ermsg,
            "cmsfs_umount(): CMS superblock at 0x%08X is corrupted",
            cmssuper);
        cmsfs_error(cmsfs_ermsg);
        return;
      }

    /*  if superblock is still busy,  then bail  */
    if (cmssuper->inuse != 0)
      {
        (void) sprintf(cmsfs_ermsg,
            "cmsfs_umount(): CMS superblock at 0x%08X is busy",
            cmssuper);
        cmsfs_error(cmsfs_ermsg);
        return;
      }

    /*  might also check for a non-NULL dir inode pointer  */

    /*
     *  VFS doesn't free inodes and superblock in the order I expected.
     *  It calls  cmsfs_put_super()  first,  and then clears inodes.
     *  The last inode cleared is root,  which is as expected.
     *  But when should the superblock be cleared and freed?
     *
     *  Since VFS will do a  cmsfs_clear_inode()  for the directory,
     *  DO NOT call it here,  and DO NOT clear the CMS dir inode here.
     */

    /*  if we still have a CMS dir inode,  go ahead and sanity check  */
    if (cmssuper->cmsrooti != NULL)
      {
        struct inode * in;
        in = cmssuper->cmsrooti->vfsinode;
        if (in->u.generic_ip != cmssuper->cmsrooti)
          {
            (void) sprintf(cmsfs_ermsg,
                "cmsfs_umount(): CMS dir inode at 0x%08X is corrupted",
                cmssuper->cmsrooti);
            cmsfs_error(cmsfs_ermsg);
            return;
          }

      }

    /*  free the CMSSUPER struct and mark it freed  */
    lock_super(sb);
    cmssuper->vfssuper = NULL;
    cmsfs_free(cmssuper);       /*  deref'd from sb->u.generic_sbp  */
    sb->u.generic_sbp = NULL;
    unlock_super(sb);

    /*  one less instance of this driver  */
    MOD_DEC_USE_COUNT;
    /*  don't do this until all storage is freed  */

    return;
  }

/* ----------------------------------------------------- CMSFS_PUT_SUPER
 *  Ordinarily a no-op because CMS FS is read-only.
 *  Cannot be used as the code path to free the superblock
 *  because of the order in which VFS comes down when unmounting.
 */
void cmsfs_put_super(struct super_block * sb)
  {
#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,"cmsfs_put_super(0x%08X) -- NOP",sb);
    cmsfs_error(cmsfs_ermsg);
#endif
    return;
  }

/* ------------------------------------------------- CMSFS_NOTIFY_CHANGE
 */
int cmsfs_notify_change(struct dentry * de,struct iattr * ia)
  {
    cmsfs_error("cmsfs_notify_change() -- N/A");
    return 0;
  }

/* -------------------------------------------------------- CMSFS_STATFS
 *  Arguments: superblock, user-space statfs, length
 *  Returns zero for success, non-zero otherwise (typically -EFAULT).
 */
int cmsfs_statfs(struct super_block * sb,struct statfs *st,int stl)
  {
    /*  EXT2 allocates a temporary statfs structure and then          *
     *  copies to user space,  limitting at stl.   We do the same.    */
    CMSSUPER  *cmssuper;
    struct  statfs  stt;
    int         rc;

#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,
        "cmsfs_statfs(0x%08X,0x%08X,%d)",sb,st,stl);
    cmsfs_error(cmsfs_ermsg);
#endif

    /*  what if superblock pointer is NULL?  */
    if (sb == NULL)
      {
        cmsfs_error("cmsfs_statfs(): NULL VFS superblock pointer");
        return -CMSFS_EFAULT;
      }

    /*  dereference CMS superblock for short-hand below  */
    cmssuper = (CMSSUPER *) sb->u.generic_sbp;

    /*  what if CMS superblock pointer is NULL?  */
    if (cmssuper == NULL)
      {
        cmsfs_error("cmsfs_statfs(): NULL CMS superblock pointer");
        return -CMSFS_EFAULT;
      }

    /*  as always,  sanity check the cross-link  */
    if (cmssuper->vfssuper != sb)
      {
        cmsfs_error("cmsfs_statfs(): corrupted CMS superblock");
        return -CMSFS_EFAULT;
      }

    /*  take data from CMS superblock  */
    stt.f_bsize = cmssuper->blksz;
    stt.f_blocks = cmssuper->blocks;
    stt.f_bfree = cmssuper->bkfree;
    stt.f_bavail = cmssuper->bkfree;
    stt.f_files = cmssuper->files;
/*  stt.f_ffree = cmssuper->ffree;  */
    stt.f_ffree = 0;            /*  no files available on R/O media  */

    /*  copy contents of our temp statfs to user space  */
    rc = copy_to_user(st, &stt, stl);
    if (rc != 0) return rc;

    return 0;
  }

/* ------------------------------------------------------- CMSFS_REMOUNT
 *  There is no remounting until Linux/390 can flush its buffers.
 */
int cmsfs_remount(struct super_block * sb,int *flags,char *data)
  {
    cmsfs_error("cmsfs_remount() -- NOP");
    /*  might be a convenient way to report FYI data on a CMS volume  */
    return 0;
  }

/* ------------------------------------------------------------------ */
static
struct super_operations cmsfs_super_operations = {
        cmsfs_read_inode,
        NULL,   /* cmsfs_write_inode, */
        NULL,   /* cmsfs_put_inode, */
        NULL,   /* cmsfs_delete_inode, */
        cmsfs_notify_change,
        cmsfs_put_super,
        NULL,   /* cmsfs_write_super, */
        cmsfs_statfs,
        cmsfs_remount,
        cmsfs_clear_inode,
        cmsfs_umount
  };

/* --------------------------------------------------------- CMSFS_MOUNT
 */
struct super_block *
cmsfs_mount(struct super_block * sb, void * data, int silent)
  {
/*  struct buffer_head * bh;  */
    struct  CMSSUPER  *cmssuper;
    int         rc;
    struct  inode  *vfsrooti;

    /*  the kernel has supplied us with a virgin superblock  */
#ifdef  CMSFS_DEBUG
    (void) sprintf(cmsfs_ermsg,
                "cmsfs_mount(0x%08X,0x%08X,%d)",sb,data,silent);
    cmsfs_error(cmsfs_ermsg);
    if (data != NULL) cmsfs_error(data);
#endif

    /*  let the kernel know we're busy ... for the moment  */
    MOD_INC_USE_COUNT;
    lock_super(sb);
#ifdef  CMSFS_DEBUG
    cmsfs_error("cmsfs_mount(): superblock now locked.");
    (void) sprintf(cmsfs_ermsg,"cmsfs_mount(): mounting %s (%04X)",
                kdevname(sb->s_dev),sb->s_dev);
    cmsfs_error(cmsfs_ermsg);
#endif

/* set device read-only (0 = read-write) */
/* BLKROSET */
/* re-read partition table */
/* BLKRRPART */
/* return device size */
/* BLKGETSIZE */

    /* -------------------------------------------------------------- *
     *  Fill-out the VFS superblock and allocate a CMS superblock.    *
     * -------------------------------------------------------------- */
    cmssuper = cmsfs_malloc(sizeof(struct CMSSUPER));
    if (cmssuper == NULL)
      {
        cmsfs_error("cannot allocate CMS superblock");
        sb->s_dev = 0;  /*  why?  */
        unlock_super(sb);
        MOD_DEC_USE_COUNT;
        return NULL;
      }

    /*  cross-reference CMS superblock and VFS superblock  */
    sb->u.generic_sbp = cmssuper;       /*  VFS points to CMS  */
    cmssuper->vfssuper = sb;            /*  CMS points to VFS  */

#ifdef  CMSFS_DEBUG
    cmsfs_error(
        "cmsfs_mount(): VFS and CMS superblocks allocated and linked");
#endif

    /*  root inode not there yet  */
    cmssuper->cmsrooti = NULL;
    /*  cmsfs_map_ADT()  will see the NULL and give us one  */

    /*  determine device blocksize  */
    cmssuper->pbksz = get_hardblocksize(sb->s_dev);
    /*  This may be redundant.  */
    set_blocksize(sb->s_dev,cmssuper->pbksz);
    /*  Got the following suggestion from Alan Cox.  */
    sb->s_blocksize_bits = 12;  /*  probably should switch()  */

    /*
     *  Is there a generic VFS super_operations struct?
     *  Because this is not cleared before we get called.
     *  So we're overwriting something ... but what?
     */
    sb->s_op = &cmsfs_super_operations;

    /*  possibly correct the VFS superblock blocksize value  */
    if (sb->s_blocksize != cmssuper->pbksz)
      {
        /*  set filesystem blocksize to device blocksize  */
        sb->s_blocksize = get_hardblocksize(sb->s_dev);
#ifdef  CMSFS_DEBUG
        (void) sprintf(cmsfs_ermsg,
            "cmsfs_mount(): s_blocksize=%d (corrected)",
            sb->s_blocksize);
        cmsfs_error(cmsfs_ermsg);
#endif
      }

    /* -------------------------------------------------------------- *
     *  Map the CMS volume (ADT) to CMS superblock.                   *
     * -------------------------------------------------------------- */
    rc = cmsfs_map_ADT(cmssuper);
    if (rc != 0)
      {
        sprintf(cmsfs_ermsg,
                "cmsfs_mount(): cmsfs_map_ADT() returned %d",rc);
        cmsfs_error(cmsfs_ermsg);
        cmsfs_free(cmssuper);
        sb->s_dev = 0;  /*  why?  */
        unlock_super(sb);
        MOD_DEC_USE_COUNT;
        return NULL;
      }

    /* -------------------------------------------------------------- *
     *  Check CMS directory inode allocated in the previous section.  *
     * -------------------------------------------------------------- */
    if (cmssuper->cmsrooti == NULL)
      {
        cmsfs_error("cmsfs_mount(): did not allocate CMS dir inode");
        cmsfs_free(cmssuper);   /*  cmssuper == sb->u.generic_sbp  */
        sb->u.generic_sbp = NULL;
        sb->s_dev = 0;  /*  why?  */
        unlock_super(sb);
        MOD_DEC_USE_COUNT;
        return NULL;
      }
    if (cmssuper->cmsrooti->cmssuper != cmssuper)
      {
        cmsfs_error("cmsfs_mount(): CMS directory inode mis-alloc'd");
        cmsfs_free(cmssuper);   /*  cmssuper == sb->u.generic_sbp  */
        sb->u.generic_sbp = NULL;
        sb->s_dev = 0;  /*  why?  */
        unlock_super(sb);
        MOD_DEC_USE_COUNT;
        return NULL;
      }

#ifdef  CMSFS_DEBUG
    cmsfs_error(
        "cmsfs_mount(): ADT mapped into CMS flavor superblock");
#endif

    /* -------------------------------------------------------------- *
     *  Allocate and flesh-out a VFS inode for the directory.         *
     *  This will be called "root" for this filesystem,               *
     *  even though there will be no other directories.               *
     *  This will be inode zero,  and is always the first FST.        *
     * -------------------------------------------------------------- */
    vfsrooti = iget(sb,0);
    if (vfsrooti == NULL)
      {
        cmsfs_error("cmsfs_mount(): cannot get directory (root) inode");
        cmsfs_free(cmssuper);   /*  cmssuper == sb->u.generic_sbp  */
        sb->u.generic_sbp = NULL;
        sb->s_dev = 0;  /*  why?  */
        unlock_super(sb);
        MOD_DEC_USE_COUNT;
        return NULL;
      }

    /*
     *  Sanity check against iget()'s work:  it should have called
     *  cmsfs_read_inode(),  which in turn should have cross-linked
     *  the CMS and VFS inode structs and marked "inode zero" as root.
     */
    if (cmssuper->cmsrooti->vfsinode != vfsrooti)
      {
        cmsfs_error("cmsfs_mount(): iget() returned a bad inode");
        cmsfs_error("cmsfs_mount(): VFS references do not match");
        cmsfs_free(cmssuper);   /*  cmssuper == sb->u.generic_sbp  */
        sb->u.generic_sbp = NULL;
        sb->s_dev = 0;  /*  why?  */
        unlock_super(sb);
        MOD_DEC_USE_COUNT;
        return NULL;
      }
    if (vfsrooti->u.generic_ip != cmssuper->cmsrooti)
      {
        cmsfs_error("cmsfs_mount(): iget() returned a bad inode");
        cmsfs_error("cmsfs_mount(): CMS references do not match");
        cmsfs_free(cmssuper);   /*  cmssuper == sb->u.generic_sbp  */
        sb->u.generic_sbp = NULL;
        sb->s_dev = 0;  /*  why?  */
        unlock_super(sb);
        MOD_DEC_USE_COUNT;
        return NULL;
      }

#ifdef  CMSFS_DEBUG
    cmsfs_error(
        "cmsfs_mount(): VFS and CMS root inodes allocated and linked");
#endif

    /* -------------------------------------------------------------- *
     *  Allocate and flesh-out a directory entry (VFS dnode).         *
     * -------------------------------------------------------------- */
    sb->s_root = d_alloc_root(cmssuper->cmsrooti->vfsinode,NULL);
    if (sb->s_root == NULL)
      {
        cmsfs_error(
            "cmsfs_mount(): cannot allocate VFS directory structure");
        cmsfs_free(cmssuper->cmsrooti->vfsinode);
        cmsfs_free(cmssuper->cmsrooti);
        cmssuper->cmsrooti = NULL;
        cmsfs_free(cmssuper);   /*  cmssuper == sb->u.generic_sbp  */
        sb->u.generic_sbp = NULL;
        sb->s_dev = 0;  /*  why?  */
        unlock_super(sb);
        MOD_DEC_USE_COUNT;
        return NULL;
      }
#ifdef  CMSFS_DEBUG
    cmsfs_error("cmsfs_mount(): allocated VFS directory structure");
#endif

    /*  CMS FS mounts are always read-only to Linux  */
    sb->s_flags |= MS_RDONLY ;
    sb->s_flags |= MS_NOSUID ;

    unlock_super(sb);
#ifdef  CMSFS_DEBUG
    cmsfs_error("cmsfs_mount(): superblock now UNlocked. Done!");
#endif

    return sb;
  }

/* ------------------------------------------------------------------ */
static
struct file_system_type cms_fs_type = {
        "cms" ,
/*      FS_REQUIRES_DEV | FS_NO_DCACHE | FS_NO_PRELIM ,    */
        FS_REQUIRES_DEV ,
        cmsfs_mount ,
        NULL
        } ;

/* ---------------------------------------------------------- CMSFS_INIT
 *  filesystem init, especially useful for module init
 */
int cmsfs_init()
  {
#ifdef  CMSFS_DEBUG
    cmsfs_error("cmsfs_init()");
#endif
    (void) sprintf(cmsfs_ermsg,"%s %s",CMSFS_DESCRIPTION,CMSFS_VERSION);
    cmsfs_error(cmsfs_ermsg);
#ifdef  CONFIG_PROC_FS
        rpc_register_sysctl();
        rpc_proc_init();
/*
        rpc_proc_register(&cmsfs_rpcstat);
 */
#endif
    return register_filesystem(&cms_fs_type);
  }

/* ------------------------------------------------------- CMSFS_CLEANUP
 *  filesystem cleanup & shut down for module extraction
 */
void cmsfs_cleanup()
  {
#ifdef  CMSFS_DEBUG
    cmsfs_error("cmsfs_cleanup()");
#endif
#ifdef  CONFIG_PROC_FS
        rpc_proc_unregister("cms");
#endif
    unregister_filesystem(&cms_fs_type);
    return;
  }


/* -------------------------------------------------------- CMSFS_MODULE
 *  additional routines required when CMS FS is built as a module
 */

#ifdef  MODULE

MODULE_AUTHOR(CMSFS_AUTHOR);
MODULE_DESCRIPTION(CMSFS_DESCRIPTION);
/*
MODULE_SUPPORTED_DEVICE(name)
MODULE_PARM(var,type)
MODULE_PARM_DESC(var,desc)
 */

EXPORT_NO_SYMBOLS;

int init_module() { return cmsfs_init(); }
void cleanup_module() { cmsfs_cleanup(); }

/*  {
module.name = "cmsfs";
module.init = &cmsfs_init();
module.cleanup = &cmsfs_cleanup();
    }  */

#endif  /* MODULE */

