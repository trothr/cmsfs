/*
 *
 *        Name: cmsfscp.c
 *      Author: Rick Troth, BMC Software, Inc., Houston, Texas, USA
 *        Date: 2000-Oct-17 (Tue)
 *
 *              Mostly like  'cmsfscat'  but preserves time stamps.
 *              cmsfs_read() interpolates NL into the stream for us
 *              so our work in this module is nicely limitted to
 *              open(), read(), write(), and finally utime().
 *
 *

-a      ASCII (text), translate the file and add NL (LF)

-i      IMAGE (binary), dump the file as a stream of bytes

-v      VARREC (raw), dump the file with any
        record length information included in-band

 *
 *
 */

#include        <fcntl.h>
#include        <utime.h>
#include        <malloc.h>
#include        <sys/stat.h>
#include        <stdio.h>
#include        <string.h>

#include        "cmsfs.h"

#define  USAGE  "Usage: %s [ -d cmsvol ] [ -a | -i ] cmsfile unixfile\n"

int main(int argc,unsigned char *argv[])
  {
    struct  utimbuf  stdtime;
    struct  CMSINODE  *cmsfile;         /*  CMS file handle  */
    int         stdfile;                /*  Unix file handle  */
    int         cmsflags, stdflags;
    unsigned  char  *source, *target, *buffer, *devname,
                stdname[256], *sbname, *_bname;
    int         buflen;
    int         rc, i, argl;
    struct  stat  targstat;

    cmsflags = CMSFSANY;
    stdflags = O_RDWR|O_CREAT|O_TRUNC;
    source = target = devname = (unsigned char *) "";
    /*  DRAT COMPILERS THAT DON'T GROK UNSIGNED CHAR!  */

    /*  process options  */
    for (i = 1 ; i < argc && argv[i][0] == '-' ; i++)
      {
        switch (argv[i][1])
          {
            case '?':   case 'h':   case 'H':   /*  quickie help  */
        (void) fprintf(stderr,"%s %s\n",CMSFS_DESCRIPTION,CMSFS_VERSION);
        (void) fprintf(stderr,"Copy CMS files, preserving time stamps.\n");
        (void) fprintf(stderr,USAGE,argv[0]);
                        return 0;
            case 'd':
            case 'f':   devname = argv[++i] ;
                        break;
            case 'a':   /* ASCII (plain text) */
                        cmsflags |= CMSFSTXT;
                        break;
            case 'i':   /* binary (IMAGE) */
                        cmsflags |= CMSFSBIN;
                        break;
            case 'v':   /* "variable" records in the raw */
                        /*  NOT YET IMPLEMENTED  */
                        break;
            default:
                        break;
          }
      }
    if ((cmsflags & CMSFSTXT) && (cmsflags & CMSFSBIN))
      {
        (void) fprintf(stderr,"%s: conflicting options\n",argv[0]);
        (void) fprintf(stderr,USAGE,argv[0]);
        return 24;
      }

    /*  sanity check:  must specify exactly two files -or-
        must specify at least one file and a target directory  */
    argl = argc - 1;
    if ((argc - i) < 1)
      {
        (void) fprintf(stderr,"Please specify a CMS file to read.\n");
        (void) fprintf(stderr,USAGE,argv[0]);
        return 24;
      }
    if ((argc - i) < 2)
      {
        (void) fprintf(stderr,"Please specify a Unix file to write.\n");
        (void) fprintf(stderr,USAGE,argv[0]);
        return 24;
      }

    /*  target practice:  if more than three args,
        then the last arg (the "target") must be a directory  */
    target = argv[argc-1];
    rc = stat((char*)target,&targstat);

    /*  if stat() failed,  then file probably does not exist,
        and we consider it a "regular file" yet to be created  */
    if (rc != 0) targstat.st_mode = S_IFREG;

    if ((argc - i) > 2 && !S_ISDIR(targstat.st_mode))
      {
        (void) fprintf(stderr,"Last argument must be a directory.\n");
        (void) fprintf(stderr,USAGE,argv[0]);
        return 24;
      }

    /*  for each source file ...  */
    for ( ; i < argl ; i++ )
      {
        source = argv[i];

        /*  open the source file (CMS file)  */
        cmsfile = cmsfs_open(source,cmsflags,devname);
        if (cmsfile == NULL) return -1;

        /*  target is file or directory?  */
        if (S_ISDIR(targstat.st_mode))
          {
            /*  extract "basename" from source string  */
            sbname = _bname = source;
            while (*_bname != 0x00)
              {
                if (*_bname == '/') sbname = _bname;
                _bname++;
              }
            if (*sbname == '/') sbname++;
            if (strlen(target) + strlen(sbname) > 254)
              {
                (void) fprintf(stderr,"filename too long\n");
                return -1;
              }
            (void) sprintf((char*)stdname,"%s/%s",target,sbname);
          }
        else
          {
            if (strlen(target) > 255)
              {
                (void) fprintf(stderr,"filename too long\n");
                return -1;
              }
            (void) sprintf((char*)stdname,"%s",target);
          }

        /*  open the target file (Unix file)  */
        stdfile = open((char*)stdname,stdflags,0666);
        if (stdfile < 0)
          {
            (void) perror((char*)stdname);
            return -1;
          }

        /*  note the time stamp on the source file  */
        stdtime.actime = stdtime.modtime = cmsfile->ctime;

        /*  allocate a buffer to hold the largest possible  "record"  */
        buflen = cmsfile->lrecl + 2;    /*  plus a little to spare  */
        buffer = (unsigned char *) malloc(buflen);
        if (buffer == NULL) return -1;

        /*  copy all records from source to target  */
        while (1)
          {
            /*  read a record  */
            rc = cmsfs_read(cmsfile,buffer,buflen);
            if (rc <= 0) break;
            /*  write the data  */
            rc = write(stdfile,buffer,rc);
            if (rc <= 0) break;
          }

        /*  was there an error?  */
        if (rc < 0)
          {
            (void) cmsfs_close(cmsfile);
            (void) close(stdfile);
            return rc;
          }

        /*  close files,  ignoring return codes  */
        (void) cmsfs_close(cmsfile);
        (void) close(stdfile);

        /*  stamp the file with the mod time of the CMS source file  */
        rc = utime((char*)stdname,&stdtime);
        if (rc != 0)
          {
            (void) perror((char*)stdname);
            return rc;
          }
      }

    return 0;
  }


