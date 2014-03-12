/*
 *
 *        Name: cmsfscat.c
 *      Author: Rick Troth, BMC Software, Inc., Houston, Texas, USA
 *        Date: 2000-Jun-26 (Mon)
 *
 *              cmsfs_read() interpolates NL into the stream for us
 *              so our work in this module is minimized to
 *              open(), read(), write(), and finally utime().
 *

-a      ASCII (text), translate the file and add NL (LF)

-i      IMAGE (binary), dump the file as a stream of bytes

-v      VARREC (raw), dump the file with any
        record length information included in-band

 *
 *
 */

#include        <stdlib.h>
#include        <stdio.h>
/* cheezy way to get time_t def'd */
#include        <utime.h>

#include        "cmsfs.h"

#define  USAGE  "Usage: %s [ -d cmsvol ] [ -a | -i ] cmsfile [cmsfile ...]\n"

/* ------------------------------------------------------------------ */
int main(int argc,unsigned char *argv[])
  {
    int         i, rc, j, k, l, catflags;
    unsigned char *devname, *filename;
    unsigned  char  *buffer;

    struct  CMSINODE  *cmsfile;         /*  CMS file handle  */
    int         buflen;

    /*  default empty strings  */
    devname = filename = (unsigned char *) "";
    /*  DRAT COMPILERS THAT DON'T GROK UNSIGNED CHAR!  */
    catflags = CMSFSANY;

    /*  process options  */
    for (i = 1 ; i < argc && argv[i][0] == '-' ; i++)
      {
        switch (argv[i][1])
          {
            case '?':   case 'h':   case 'H':   /*  quickie help  */
        (void) printf("%s %s\n",CMSFS_DESCRIPTION,CMSFS_VERSION);
        (void) printf("Concatenate CMS files to stdout.\n");
        (void) printf(USAGE,argv[0]);
                        return 0;
            case 'd':
            case 'f':   devname = argv[++i] ;
                        break;
            case 'a':   /* ASCII (plain text) */
                        catflags |= CMSFSTXT;
                        break;
            case 'i':   /* binary (IMAGE) */
                        catflags |= CMSFSBIN;
                        break;
            case 'v':   /* "variable" records in the raw */
                        /*  NOT YET IMPLEMENTED  */
                        break;
            default:
                        break;
          }
      }
    if ((catflags & CMSFSTXT) && (catflags & CMSFSBIN))
      {
        (void) fprintf(stderr,"%s: conflicting options\n",argv[0]);
        (void) fprintf(stderr,USAGE,argv[0]);
        return 24;
      }

    /*  sanity check  */
    if (argc <= i)
      {
        (void) printf("Please specify a CMS file to read.\n");
        (void) printf(USAGE,argv[0]);
        return 24;
      }

    /*  for each source file ...  */
    for ( ; i < argc ; i++ )
      {
        filename = argv[i];

        /*  open the source file (CMS file)  */
        cmsfile = cmsfs_open(filename,catflags,devname);
        if (cmsfile == NULL) return -1;

        /*  allocate a buffer to hold the largest possible  "record"  */
        buflen = cmsfile->lrecl + 2;    /*  plus a little to spare  */
        buffer = malloc(buflen);      /*  (you just gotta know that)  */
        if (buffer == NULL) 
	  { 
	    (void) perror("malloc()"); 
	    return -1;
	  } 

        /*  copy all records from source to target  */
        while (1)
          {
            /*  read a record  */
            rc = cmsfs_read(cmsfile,buffer,buflen);
            if (rc <= 0) break;
            /*  write the data  */
            rc = write(1,buffer,rc);
            if (rc <= 0) break;
          }

        /*  was there an error?  */
        if (rc < 0)
          {
            (void) cmsfs_close(cmsfile);
            return rc;
          }

        /*  close files,  ignoring return codes  */
        (void) cmsfs_close(cmsfile);
      }

    /*  exit  */
    return 0;
  }


