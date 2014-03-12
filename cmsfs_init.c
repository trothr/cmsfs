/*
 * linux/fs/cms/cmsfs_init.c
 *
 * Taken from msdosfs_syms.c. 
 */

extern void cmsfsini(); 

/* 
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/cms_fs.h>
 */ 
#include "cmsfs.h"
#include <linux/fs.h>

/* static? */ 
struct file_system_type cms_fs_type = {
	cmsfsini, "cms", 1, NULL
};


/* --------------------------------------------------------- INIT_CMS_FS 
 */ 
int init_cms_fs() 
  {
    int 	rc; 

    rc = register_filesystem(&cms_fs_type); 
    if (rc != 0) return rc; 
 
    return 0; 
  } 
 

/* --------------------------------------------------------------------- 
 */ 
struct super_block * 
cmsfsini (struct super_block * sb, void * data, int silent)
{ 
return NULL; 
} 



