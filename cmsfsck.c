/*
 *
 *        Name: cmsfsck.c (C source)
 *              'fsck' for CMS filesystems
 *
 *        Note: Common Linux practice is to copy or rename
 *              the executable so that it is known as 'fsck.cms'.
 *              This is not strictly required for "utility mode" CMS FS.
 */

#include        <time.h>
/* the following is a cheezy way to get ssize_t def'd */
#include        <fcntl.h>
#include        <stdio.h>

#include        "cmsfs.h"

#define  USAGE  "Usage: %s cmsvol [ cmsvol ... ] \n"

int main(int argc,unsigned char *argv[])
  {
    int         i, j, k, rc, percent;
    struct  CMSSUPER  *tempvols;

    /*  process options  */
    for (i = 1 ; i < argc && argv[i][0] == '-' ; i++)
      {
        switch (argv[i][1])
          {
            case '?':   case 'h':   case 'H':   /*  quickie help  */
        (void) fprintf(stderr,"%s %s\n",CMSFS_DESCRIPTION,CMSFS_VERSION);
        (void) fprintf(stderr,"'fsck' for CMS volumes.\n");
        (void) fprintf(stderr,USAGE,argv[0]);
                        return 0;
            case 'd':
            case 'f':   /* devname = argv[++i] ; */
                        /*  (see loop below;  handled differently now)  */
                        break;
/* "Automatically  repair  the  file system without any questions ..." */
            case 'a':   /*  we don't repair anyway;  just ignore it  */
                        break;
            default:
                        break;
          }
      }

    /*  sanity check  */
    if (argc <= i)
      {
        (void) fprintf(stderr,"Please specify a CMS volume to check.\n");
        (void) fprintf(stderr,USAGE,argv[0]);
        return 24;
      }

    while (i < argc)
      {
        /*  'mount'  it to validate the ADT and directory FST  */
        tempvols = cmsfs_vopen(argv[i]);
        if (tempvols == NULL)
          {
            (void) fprintf(stderr,"'%s' is not a CMS volume.\n",argv[i]);
            return -1;
          }

        /*  'umount'  to cleanly release resources  */
        rc = cmsfs_vclose(tempvols);
        if (rc != 0) return rc;

        i++;
      }

    return 0;
  }


