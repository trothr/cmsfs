/*
 *
 *	  Name: cmsfsvfs.c (C source)
 *	  Date: 2000-Sep-14 (Thu) 
 *		split from other CMS FS components 
 *
 *		This source contains the routines and structures 
 *		required when CMS FS is built as a driver. 
 *		It interfaces between CMS FS' own structures, 
 *		which read the CMS EDF format,  and Linux VFS. 
 */ 
 
#define 	__KERNEL__ 
#define 	MODULE 
 
#include	<linux/module.h> 
#include	<linux/version.h> 
 
/* #include	<linux/types.h> */ 
#include	<linux/fs.h> 
#include	<linux/mm.h> 
 
#include	"cmsfs.h" 
/* #include	"aecs.h" */ 
 
#include	"cmsfsany.c" 
 
/*  I learned of this requirement from Neale's "cpint" code.  */ 
char kernel_version [] = UTS_RELEASE; 
 
/* -------------------------------------------------------- CMSFS_KERNEL 
 *  routines required when CMS FS is built as a driver 
 * 
 *  CMS FS inode numbers are base-zero index into the directory. 
 *  CMS FS (EDF) is flat, no sub-directories, so there is 
 *  only one such directory into which we index. 
 */ 
void cmsfs_read_inode(struct inode * in) 
  { 
    (void) printk("CMSFS: cmsfs_read_inode()\n"); 
    return; 
  } 
 
void cmsfs_put_super(struct super_block * sb) 
  { 
    (void) printk("CMSFS: cmsfs_put_super()\n"); 
    return; 
  } 
 
int cmsfs_statfs(struct super_block * sb,struct statfs *st,int stl) 
  { 
    /*  EXT2 allocates a temporary statfs structure 
	and then copies the contents,  limitting at stl  */ 
 
    (void) printk("CMSFS: cmsfs_statfs()\n"); 
 
/* 
    st->f_bsize = cmssuper->blksz; 
    st->f_blocks = cmssuper->blocks; 
    st->f_bfree = cmssuper->bkfree; 
    st->f_bavail = cmssuper->bkfree; 
    st->f_files = cmssuper->files; 
 
    st->f_ffree = cmssuper->ffree; 
 */ 
 
    return 0; 
  } 
 
static 
struct super_operations cmsfs_operations = { 
        cmsfs_read_inode, 
	NULL,	/* cmsfs_write_inode, */ 
	NULL,	/* cmsfs_put_inode, */ 
	NULL,	/* cmsfs_delete_inode, */ 
	NULL,	/* cmsfs_notify_change, */ 
        cmsfs_put_super, 
	NULL,	/* cmsfs_write_super, */ 
        cmsfs_statfs, 
	NULL,	/* cmsfs_remount, */ 
	NULL,	/* cmsfs_clear_inode, */ 
	NULL	/* cmsfs_umount */ 
  }; 
/*  mimic ISO, which is also read-only media  */ 
/*  mimic MS-DOS, which has similar short names (8x3)  */ 
 
/* --------------------------------------------------------- CMSFS_MOUNT 
 */ 
struct super_block * 
cmsfs_mount(struct super_block * sb, void * data, int silent) 
  { 
    struct buffer_head * bh;
    struct  CMSVOLID  *cmssuper; 
 
    /*  the kernel has supplied us with a virgin superblock  */ 
    (void) printk("CMSFS: cmsfs_mount()\n"); 
 
    /*  let the kernel know we're busy ... for the moment  */ 
    MOD_INC_USE_COUNT; 
/*  (void) lock_super(sb);  */ 
 
    (void) printk("CMSFS: sb=%08X, data=%08X, silent=%d\n",sb,data,silent); 
    (void) printk("CMSFS: s_dev=%s\n",kdevname(sb->s_dev)); 
    (void) printk("CMSFS: blocksize=%d\n",get_hardblocksize(sb->s_dev));
 
    (void) printk("CMSFS: s_dev=%s (%04X)\n",kdevname(sb->s_dev),sb->s_dev); 
    (void) printk("CMSFS: s_blocksize=%d\n",sb->s_blocksize); 
    (void) printk("CMSFS: s_type=%s\n",sb->s_type); 
    (void) printk("CMSFS: s_root=%08X\n",sb->s_root); 
    (void) printk("CMSFS: s_op=%08X\n",sb->s_op); 
 
/* 
    sb->s_type = 
 */ 
 
    /*  careful! this is not cleared before we get called  */ 
    sb->s_op = &cmsfs_operations; 
    (void) printk("CMSFS: s_op=%08X (after)\n",sb->s_op); 
 
    sb->s_blocksize = get_hardblocksize(sb->s_dev); 
    (void) printk("CMSFS: s_blocksize=%d (after)\n",sb->s_blocksize); 
 
    /* -------------------------------------------------------------- * 
     *  Map the CMS volume (ADT) ...                                  * 
     * -------------------------------------------------------------- */ 
    cmssuper = (void*) kmalloc(sizeof(struct CMSVOLID), GFP_KERNEL);
    if (cmssuper == NULL) 
      { 
	(void) printk ("CMSFS: cannot allocate CMS superblock!\n"); 
	sb->s_dev = 0;	/*  why?  */ 
/*	(void) unlock_super (sb);  */ 
	MOD_DEC_USE_COUNT;
	return NULL; 
      } 
    sb->u.generic_sbp = cmssuper; 
 
 
 
    /* -------------------------------------------------------------- * 
     *  Map the CMS directory (FST) ...                               * 
     * -------------------------------------------------------------- */ 
 
    /* -------------------------------------------------------------- * 
     *  Allocate and flesh-out an inode for the directory.            * 
     *  This will be called "root" for this filesystem,               * 
     *  even though there will be no other directories.               * 
     * -------------------------------------------------------------- */ 
/* 
    root_inode = iget(s, MINIX_ROOT_INO);
    if (!root_inode)
      { 
	(void) printk ("CMSFS: cannot get root inode!\n"); 
	kfree(sb->u.generic_sbp); 
        sb->s_dev = 0; 
	(void) unlock_super (sb); 
	(void) brelse(bh); 
	MOD_DEC_USE_COUNT; 
	return NULL; 
      } 
 
        s->s_root = d_alloc_root(root_inode, NULL);
        if (!s->s_root) 
                goto out_iput;   
 
        s->s_root->d_op = &minix_dentry_operations;
 */ 
 
/* 
    ->vfsinode = iget(sb,0);	**  or get_empty_inode()?  ** 
    sb->s_root = d_alloc_root(->vfsinode,NULL); 
 */ 
    sb->s_root = NULL; 
 
/*
    if (!sb->s_root) 
      { 
	(void) printk ("CMSFS: cannot read directory!\n"); 
	kfree(sb->u.generic_sbp); 
	sb->s_dev = 0; 
	(void) unlock_super (sb); 
	(void) brelse(bh); 
	MOD_DEC_USE_COUNT; 
	return NULL; 
      } 
 */ 
/* 
    sb->s_root->i_sb = sb; 
    sb->s_root->i_dev = sb->s_dev; 
 */ 
/* 
        root_inode->i_ino = MSDOS_ROOT_INO;
 */ 
 
/* 
    sb->s_root->i_mtime = 
    sb->s_root->i_atime = 
    sb->s_root->i_ctime = 0;
 */ 
    /*  the volume also has a time stamp, just FYI  */ 
 
/* 
    sb->s_root->i_nlink = 2; 
 */ 
 
/* 
    sb->s_root->i_op = 
 */ 
/*  inode operations for the directory  */ 
 
    return sb; 
/*  kfree(sb->u.generic_sbp);  */ 
/*  (void) unlock_super (sb);  */ 
/*  (void) brelse(bh);  memory leak here  */ 
/*  MOD_DEC_USE_COUNT;  */ 
/*  return NULL;  */ 
  } 
 
/* ------------------------------------------------------------------ */ 
static 
struct file_system_type cms_fs_type = { 
	"cms", 
	FS_REQUIRES_DEV | FS_NO_DCACHE, 
	cmsfs_mount, 
        NULL 
	}; 
 
/* ---------------------------------------------------------- CMSFS_INIT 
 *  filesystem init, especially useful for module init 
 */ 
int cmsfs_init() 
  { 
    (void) printk("CMSFS: cmsfs_init()\n"); 
#ifdef	CONFIG_PROC_FS 
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
    (void) printk("CMSFS: cmsfs_cleanup()\n"); 
#ifdef	CONFIG_PROC_FS 
	rpc_proc_unregister("cms"); 
#endif 
    unregister_filesystem(&cms_fs_type); 
  } 
 
 
/* -------------------------------------------------------- CMSFS_MODULE 
 *  additional routines required when CMS FS is built as a module 
 */ 
 
#ifdef	MODULE 
 
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
 
#endif	/* MODULE */ 
 
/* --------------------------------------------------------- CMSFS_BREAD 
 *  The "b" here means "block", not "buffer".  Do not confuse this with 
 *  Linux VFS bread() function.   This is CMS FS "block read" function. 
 */ 
int cmsfs_bread(struct CMSVOLID *vol,void *buf,int block,int blocksize) 
  { 
    struct buffer_head * bh; 
    struct super_block * sb; 
 
    (void) printk("CMSFS: cmsfs_bread(-,%d,%d)\n",block,blocksize); 
 
    sb = (struct super_block *) vol->vfssuper; 
 
    bh = bread(sb->s_dev,block,blocksize); 
    if (bh == NULL) return -1; 
 
/* 
    (void) memcpy(buf,bh->b_data,blocksize);
 */ 
    (void) memmove(buf,bh->b_data,blocksize);
/*  (void) brelse(bh);  memory leak here  */ 
 
    return blocksize; 
  }
 
 
