/*
 *
 *        Name: cmsfsvol.c (C source)
 *              report CMS FS volume information like 'Q DISK'
 *
 */

#include        <time.h>
/* the following is a cheezy way to get ssize_t def'd */
#include        <fcntl.h>
#include        <stdio.h>

#include        "cmsfs.h"

#define  USAGE  "Usage: %s [ -d ] cmsvol [cmsvol ...]\n"

/* ------------------------------------------------------------------ */
int main(int argc,unsigned char *argv[])
  {
    int         i, percent;
    struct  CMSSUPER  *tempvols;
    struct  CMSINODE  dirinode, tmpinode;
    struct tm *ftime;

    /*  process options  */
    for (i = 1 ; i < argc && argv[i][0] == '-' ; i++)
      {
        switch (argv[i][1])
          {
            case '?':   case 'h':   case 'H':   /*  quickie help  */
        (void) fprintf(stderr,"%s %s\n",CMSFS_DESCRIPTION,CMSFS_VERSION);
        (void) fprintf(stderr,
"Report CMS volume info in CMS 'Q DISK' format.\n");
        (void) fprintf(stderr,USAGE,argv[0]);
                        return 0;
            case 'd':
            case 'f':   /*  devname = argv[++i];  */
                        /*  (see loop below;  handled differently now)  */
                        break;
            default:
                        break;
          }
      }

    /*  sanity check:  specify a volume to list  */
    if (i >= argc)
      {
        (void) fprintf(stderr,"Please specify a CMS volume to report.\n");
        (void) fprintf(stderr,USAGE,argv[0]);
        return 24;
      }

    /*  print a header;  looks like CMS  */
    (void) printf("LABEL  VDEV M  STAT   CYL TYPE \
BLKSZ   FILES  BLKS USED-(%) BLKS LEFT  BLK TOTAL\n");

    for (      ; i < argc ; i++)
      {
        /*  open the volume  */
        tempvols = cmsfs_vopen(argv[i]);
        if (tempvols == NULL)
          {
            (void) fprintf(stderr,"'%s' is not a CMS volume.\n",argv[i]);
            return -1;
          }

        /*  report on the volume  */
        percent = ( tempvols->bkused * 100 ) / tempvols->blocks;
        (void) printf("%-6s .... ... R/O %5d .... %4d %8d %10d-%02d %10d \
%10d\n",tempvols->volid,tempvols->ncyls,tempvols->blksz,tempvols->files,
tempvols->bkused,percent,tempvols->bkfree,tempvols->blocks);

        /*  close the volume  */
        (void) cmsfs_vclose(tempvols);
      }

    /*  exit  */
    return 0;
  }


