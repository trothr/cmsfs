/* 
 * 
 *	  Date: 2000-Jun-26 (Mon) 
 * 
 
-a	ASCII (text), translate the file and add NL (LF) 
 
-i	IMAGE (binary), dump the file as a stream of bytes 
 
-v	VARREC (raw), dump the file with any 
	record length information included in-band 
 
 * 
 * 
 */ 
 
 
/* 
#include	<fcntl.h> 
#include	<stdio.h> 
#include	<unistd.h> 
 
#include	"aecs.h" 
 */ 
 
#include	"cmsfs.h" 
 
 
int main(int argc,char *argv[]) 
  { 
    int 	i, rc; 
    char       *devname; 
    struct  CMSVOLID  tempvols; 
 
    /*  default empty strings  */ 
    devname = ""; 
 
    /*  process options  */ 
    for (i = 1 ; i < argc && argv[i][0] == '-' ; i++) 
      { 
	switch (argv[i][1]) 
	  { 
	    case 'd': 
	    case 'f':   devname = argv[++i] ; 
			break; 
	    case 'a':	/* ASCII (plain text) */ 
			break; 
	    case 'i':	/* binary (IMAGE) */ 
			break; 
	    case 'v':	/* "variable" records in the raw */ 
			break; 
	    default: 
			break; 
	  } 
      } 
 
    /*  sanity check  */ 
    if (*devname == 0x00) 
      { 
	(void) printf("Please specify a CMS volume.\n"); 
	(void) printf("Usage: %s -f cmsvol\n",argv[0]); 
	return 24; 
      } 
 
    /*  open the volume  */ 
    rc = cmsfsop1(devname,&tempvols); 
(void) printf("cmsfsop1(\"%s\",) returned %d\n",devname,rc); 
    if (rc != 0) return rc; 
 
    /*  report on the volume  */ 
    (void) printf("ADTID=%s volume label\n",tempvols.volid); 
    (void) printf("ADTDBSIZ=%d block size\n",tempvols.blksz); 
    (void) printf("ADTDOP=%d origin pointer\n",tempvols.origin); 
    (void) printf("ADTCYL=%d cylinders used\n",tempvols.ncyls); 
    (void) printf("ADTMCYL=%d cylinders max\n",tempvols.mcyls); 
    (void) printf("ADTNUM=%d blocks on disk\n",tempvols.blocks); 
    (void) printf("ADTUSED=%d blocks used\n",tempvols.bkused); 
    (void) printf("ADTFSTSZ=%d FST size\n",tempvols.fstsz); 
    (void) printf("ADTNFST=%d FSTs per block\n",tempvols.fstct); 
    (void) printf("Created %s\n",ctime(&tempvols.ctime)); 
    if (tempvols.resoff != 0) 
    (void) printf("ADTOFFST=%d reserved offset\n",tempvols.resoff); 
    (void) printf("%d blocks free\n",tempvols.bkfree); 
 
 
    /*  close the volume  */ 
/* 
    rc = cmsfscl1(&tempvols); 
(void) printf("cmsfscl1() returned %d\n",rc); 
 */ 
 
    /*  exit  */ 
    return 0; 
  } 
 
 
