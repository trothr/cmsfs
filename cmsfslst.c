/*
 *
 *        Name: cmsfslst.c (C source)
 *              list the files on a CMS FS volume
 *              in CMS 'LISTFILE' format.
 *
 */

#include        <stdio.h>
#include        <time.h>
#include        <fcntl.h>

#include        "cmsfs.h"

#define  USAGE  "Usage: %s -d cmsvol\n"

/* ------------------------------------------------------------------ */
int main(int argc,char*argv[])
  {
    int         i, j, k, rc, percent;
    unsigned char  *devname;
    struct  CMSSUPER  *tempvols;
    struct  CMSINODE  *dirinode, tmpinode;
    struct tm *ftime;

    /*  default empty strings  */
    devname = (unsigned char *) "";

    /*  process options  */
    for (i = 1 ; i < argc && argv[i][0] == '-' ; i++)
      {
        switch (argv[i][1])
          {
            case '?':   case 'h':   case 'H':   /*  quickie help  */
        (void) fprintf(stderr,"%s %s\n",CMSFS_DESCRIPTION,CMSFS_VERSION);
        (void) fprintf(stderr,"List CMS files in CMS 'LISTFILE' format.\n");
        (void) fprintf(stderr,USAGE,argv[0]);
                        return 0;
            case 'd':
            case 'f':   devname = argv[++i] ;
                        break;
            default:
                        (void) fprintf(stderr,
        "Unrecognized option '%s'\n",argv[i]);
        (void) fprintf(stderr,USAGE,argv[0]);
                        return 24;
                        break;
          }
      }

    /*  sanity check  */
    if (*devname == 0x00)
      {
        (void) fprintf(stderr,"Please specify a CMS volume.\n");
        (void) fprintf(stderr,USAGE,argv[0]);
        return 24;
      }

    /*  open the volume  */
    tempvols = cmsfs_vopen(devname);
    if (tempvols == NULL) return -1;

    /*  report on the volume  */
#ifdef  CMSFS_DEBUG
    (void) printf("LABEL  VDEV M  STAT   CYL TYPE \
BLKSZ   FILES  BLKS USED-(%) BLKS LEFT  BLK TOTAL\n");
    percent = ( tempvols->bkused * 100 ) / tempvols->blocks ;
    (void) printf("%-6s .... ... ... %5d .... %4d %8d %10d-%02d %10d %10d\n",
        tempvols->volid,tempvols->ncyls,tempvols->blksz,tempvols->files,
        tempvols->bkused,percent,tempvols->bkfree,tempvols->blocks);
#endif

    /*  open the directory  */
    dirinode = cmsfs_fopen(
        (unsigned char *)".directory",tempvols,O_RDONLY);
    if (dirinode == NULL) return -1;

    /*  sanity check:  same number of objects in directory
        as number of files in this (flat) filesystem  */
    if (dirinode->items != tempvols->files)
      {
        (void) fprintf(stderr,
            "corrupted filesystem; #files (%d) != #dir items (%d)\n",
            tempvols->files,dirinode->items);
        (void) cmsfs_fclose(dirinode);
        (void) cmsfs_vclose(tempvols);
        return -1;
      }

#ifdef  CMSFS_DEBUG
    (void) printf("FSTSZ=%d, FSTCT=%d, FILES=%d\n",
        tempvols->fstsz,tempvols->fstct,tempvols->files);
#endif

    /*  print a nice header line  */
    (void) printf("FILENAME FILETYPE FM FORMAT LRECL       \
RECS     BLOCKS     DATE     TIME\n");

    j = tempvols->fstct;
    /*  list the files  */
    for (i = k = 0 ; i < tempvols->files ; i++)
      {
        unsigned char buff[4096], *pz;
        struct CMSFSFST *pp;

        /*  first time through this loop,  index j gets zeroed  */
        if (j >= tempvols->fstct)
          {
            /*  read in a new block of FSTs  */
            (void) cmsfsrd2(dirinode,buff,k);
            /*  and reset the index and increment the accum  */
            k += 1;  j = 0;
          }

/*      pp = &buff[j*tempvols->fstsz];  */
        pz = buff;
        pz += (j * tempvols->fstsz);
        pp = (struct CMSFSFST *) pz;

#ifdef  CMSFS_DEBUG
        (void) printf("index=%d %d\n",j,j*tempvols->fstsz);
        (void) printf("pointer %08X\n",pp);
#endif

        tmpinode.cmssuper = tempvols;   /*  because this file
                                            never gets "openned"  */
        (void) cmsfs_map_FST(&tmpinode,pp);

        /*  CMS FS times are local,  even though
            "ctime" member of the inode is GMT  (offset zero)  */
        ftime = localtime(&tmpinode.ctime);

        (void) printf("%-8s %-8s %2s %1s %10d %10d %10d \
%2d/%02d/%04d %2d:%02d:%02d\n",
            tmpinode.fname,tmpinode.ftype,tmpinode.fmode,tmpinode.recfm,
            tmpinode.lrecl,tmpinode.items,tmpinode.bloks,
            ftime->tm_mon+1,ftime->tm_mday,ftime->tm_year+1900,
            ftime->tm_hour,ftime->tm_min,ftime->tm_sec);

        j += 1;
      }

    /*  close the directory  */
    (void) cmsfs_fclose(dirinode);

    /*  close the volume  */
    rc = cmsfs_vclose(tempvols);

    /*  exit  */
    return 0;
  }


