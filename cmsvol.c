/* 
 * 
 *	  Date: 2000-Jun-15 
 * 
 */ 
 
#include	<fcntl.h> 
#include	<stdio.h> 
#include	<unistd.h> 
 
#include	"cmsfs.h" 
#include	"aecs.h" 
 
int main(int argc , char *argv[]) 
  { 
    int 	fd, rc, bksz, i, n, cmsfsbk1, cmsfsbk2, cmsfsbk3; 
    char	buff[256]; 
    struct	FSTD		cmsfst, *workspace; 
    struct	CMSFSADT	cmsvol; 
    char       *p, *q; 
    int 	lrecl; 
    int 	blocks, bkused, bkfree, files, blksz; 
    char       *devname, *filename; 
    int 	origin, cyls, maxcyls; 
    struct	CMSINODE	rootinode, fileinode; 
    struct	CMSVOLID	cmsvolid; 
    char       *work; 
 
    /*  array of possible CMS FS blocksizes  */ 
    int 	cmsfsbkt[8]	=	CMSFSBKT ; 
 
    /*  char array (4 bytes) "CMS1" filesystem identifier  */ 
    char	cmsfsvsk[8]	=	CMSFSVSK ; 
    /*  (a "magic number" by any other name)  */ 
 
/* 
(void) printf("DIR struct size %d\n",sizeof(cmsfst)); 
(void) printf("VOL struct size %d\n",sizeof(cmsvol)); 
fflush(stdout); 
 */ 
 
    bksz = 0; 
    devname = "" ; 
    filename = "" ; 
 
    for (i = 1 ; i < argc && argv[i][0] == '-' ; i++) 
      { 
	switch (argv[i][1]) 
	  { 
	    case 'd': 
	    case 'f':   devname = argv[++i] ; 
			break; 
	    default: 
			break; 
	  } 
      } 
    if (i < argc) filename = argv[i]; 
 
    /*  sanity check  */ 
    if (*devname == 0x00) 
      { 
	(void) printf("please specify a filesystem.\n"); 
	return 24; 
      } 
 
/*  beyond this point, use the library  */ 
rc = cmsfsop1(devname,&cmsvolid); 
 
    /*  open the filesystem  */ 
    fd = open(devname,O_RDONLY); 
    if (fd < 0) 
      { 
	(void) perror("open()"); 
	return fd; 
      } 
 
    /*  look for the CMS volume label (aka VOLSER)  */ 
    for (i = 0 ; cmsfsbkt[i] != 0 ; i++) 
      { 
	cmsfsbk1 = cmsfsbkt[i]; 
/* 
	(void) printf("trying %d blocksize ...\n",cmsfsbk1); 
 */ 
	cmsfsbk2 = cmsfsbk1 * 2; 
	rc = lseek(fd,cmsfsbk2,SEEK_SET); 
	if (rc != cmsfsbk2) { (void) perror("lseek()"); break; } 
 
	rc = read(fd,&cmsvol,sizeof(cmsvol)); 
	if (rc != sizeof(cmsvol)) { (void) perror("read() VOL"); break; } 
 
	if (cmsvol.ADTIDENT[0] == cmsfsvsk[0] 
	 && cmsvol.ADTIDENT[1] == cmsfsvsk[1] 
	 && cmsvol.ADTIDENT[2] == cmsfsvsk[2] 
	 && cmsvol.ADTIDENT[3] == cmsfsvsk[3]) 
	  { 
	    blksz = cmsfsbex(cmsvol.ADTDBSIZ,4); 
	    if (blksz == cmsfsbk1) 
	      { 
		bksz = cmsfsbk1 ; 
		break; 
	      } 
    (void) printf("FS blocksize %d does not match disk blocksize %d\n", 
			blksz,cmsfsbk1); 
	  } 
      } 
 
    /*  did we find a CMS1 volume label?  */ 
    if (bksz == 0) 
      { 
	(void) printf("Can not find a CMS1 label on %s.\n",devname); 
	return -1; 
      } 
 
    /*  extract volume label and translate  */ 
    for (i = 0 ; i < 6 ; i++) cmsvolid.volid[i] = cmsvol.ADTID[i] ; 
    cmsvolid.volid[6] = 0x00; 
    (void) stretoa(cmsvolid.volid); 
 
    /*  extract "disk origin pointer"  */ 
    origin = cmsfsbex(cmsvol.ADTDOP,4); 
 
    /*  extract number of cylinders used  */ 
    cyls = cmsfsbex(cmsvol.ADTCYL,4); 
 
    /*  extract max number of cylinders  */ 
    maxcyls = cmsfsbex(cmsvol.ADTMCYL,4); 
 
    /*  extract "total blocks on disk" count  */ 
    blocks = cmsfsbex(cmsvol.ADTNUM,4); 
 
    /*  compute "blocks used" count  */ 
    bkused = cmsfsbex(cmsvol.ADTUSED,4); 
    bkused += 1 ;	/*  why???  */ 
 
    /*  compute "blocks free" count  */ 
    bkfree = blocks - bkused ; 
 
/* 
    (void) printf("ADTID=%s volume label\n",cmsvolid.volid); 
    (void) printf("ADTDBSIZ=%d block size\n",bksz); 
    (void) printf("ADTDOP=%d origin pointer\n",origin); 
    (void) printf("ADTCYL=%d cylinders used\n",cyls); 
    (void) printf("ADTMCYL=%d cylinders max\n",maxcyls); 
    (void) printf("ADTNUM=%d blocks on disk\n",blocks); 
    (void) printf("ADTUSED=%d blocks used\n",bkused); 
    (void) printf("ADTFSTSZ=%d FST size\n",cmsfsbex(cmsvol.ADTFSTSZ,4)); 
    (void) printf("ADTNFST=%d FSTs per block\n",cmsfsbex(cmsvol.ADTNFST,4)); 
 
    (void) printf("Created %02X-%02X-%02X %02X:%02X:%02X\n", 
	cmsvol.ADTDCRED[0],cmsvol.ADTDCRED[1],
	cmsvol.ADTDCRED[2],cmsvol.ADTDCRED[3],
	cmsvol.ADTDCRED[4],cmsvol.ADTDCRED[5]); 
    (void) printf("ADTOFFST=%d reserved offset\n",cmsfsbex(cmsvol.ADTOFFST,4)); 
 
    (void) printf("(computed) %d blocks free\n",bkfree); 
 */ 
 
    /*  origin is a "base 1" number;  convert to "base 0" offset  */ 
    cmsfsbk3 = cmsfsfom(origin,cmsfsbk1); 
 
    /*  read in the first FST  */ 
    rc = lseek(fd,cmsfsbk3,SEEK_SET); 
    if (rc != cmsfsbk3) { (void) perror("lseek()"); return rc; } 
    rc = read(fd,&cmsfst,sizeof(cmsfst)); 
    if (rc != sizeof(cmsfst)) { (void) perror("read() FST") ; return rc; } 
 
    /*  map the inode for the directory  */ 
    rc = cmsfsmap(&rootinode,&cmsfst,cmsfsbk1); 
    if (rc != 0)  { (void) perror("cmsfsmap()"); return rc; } 
 
    /*  sanity check:  FOP in dir entry must match DOP in vol  */ 
    if (cmsfsbk3 != rootinode.start) 
      { 
	(void) printf("FOP %d does not match DOP %d BS %d\n",
		rootinode.start,cmsfsbk3,cmsfsbk1); 
	(void) printf("blocksize %d levels %d\n", 
		cmsfsbk1,rootinode.level); 
	(void) lseek(fd,rootinode.start,SEEK_SET); 
	(void) read(fd,buff,32); 
	(void) cmsfsdmp(buff,32); 
	(void) lseek(fd,cmsfsbk3,SEEK_SET); 
	(void) read(fd,buff,32); 
	(void) cmsfsdmp(buff,32); 
      } 
 
    /*  sanity check:  RECFM of a directory must be "F"  */ 
    if (rootinode.recfm[0] != 'F') 
      { 
	(void) printf("directory RECFM %s not 'F'\n",rootinode.recfm); 
	return -1; 
      } 
 
    /*  sanity check:  LRECL of a directory must be 64  */ 
    if (rootinode.lrecl != 64) 
      { 
	(void) printf("directory LRECL %d not 64\n",rootinode.lrecl); 
	return -1; 
      } 
 
    /*  report  */ 
    (void) printf("Volume '%s', blksz %d, files %d, directory blocks %d\n", 
		cmsvolid.volid,bksz,rootinode.items,rootinode.bloks); 
 
    /*  load the directory  */ 
    workspace = malloc(bksz); 
rootinode.devfd = fd; 
rootinode.bksiz = bksz; 
    (void) cmsfsbkr(&rootinode,workspace,1); 
 
if (*filename != 0x00) 
(void) printf("looking for %s ...\n",filename); 
fflush(stdout); 
 
    for (i = 0; i < rootinode.items; i++) 
      { 
	if (i >= 64) break; 
	/*  map the inode for this file  */ 
	rc = cmsfsmap(&fileinode,&workspace[i],cmsfsbk1); 
	if (rc != 0)  { (void) perror("cmsfsmap()"); return rc; } 
 
if (*filename == 0x00) 
(void) printf("%s %s %d B=%d I=%d L=%d P=%d %d bytes\n",
fileinode.name,
fileinode.recfm,fileinode.lrecl,
fileinode.bloks,fileinode.items,
fileinode.level,fileinode.psize,
fileinode.items*fileinode.lrecl); 
/* 
fileinode.bloks*bksz); 
 */ 
fflush(stdout); 
 
	work = malloc(bksz); 
	fileinode.devfd = fd; 
	fileinode.bksiz = bksz; 
	rc = cmsfsbkr(&fileinode,work,1); 
	if (work == NULL) 
	  { 
	    (void) printf("something didn't work\n"); 
	    return -1; 
	  } 
 
	if (strcmp(fileinode.name,filename) == 0) 
	  { 
	    (void) printf("records=%d\n",fileinode.items); 
/*	    for (i = 1 ; i <= fileinode.items ; i++)   */ 
	    (void) cmsfsdmp(work,32); 
	  } 
 
	(void) free(work); 
      } 
 
    (void) close(fd); 
  } 
 
 
/****************************************************/ 
 
/* 
char	FSTWRPNT[2] ;      ** WRITE POINTER - ITEM NUMBER ** 
char	FSTRDPNT[2] ;      ** READ POINTER - ITEM NUMBER ** 
char	FSTFMODE[2] ;      ** FILE MODE - LETTER AND NUMBER ** 
char	FSTFCLPT[2] ;      ** FIRST CHAIN LINK POINTER ** 
char	FSTFLAGS[1] ;      ** F*2 - FST FLAG BYTE ** 
char	FSTBLKCT[2] ;      ** NUMBER OF 800 BYTE BLOCKS ** 
char	FSTREALM[1] ;      ** Real filemode ** 
char	FSTFLAG2[1] ;      ** F*3 - FST FLAG BYTE 2 FSTFLAG2 ** 
*/
 
 
 
 
/* 
-r--r--r--   1 N/A      N/A          size mon dd hh:mm fn.ft 
-r--r--r--   1 owner    group        size mon dd hh:mm fn.ft 
-r--r--r--   1 owner    group        size mon dd  yyyy fn.ft 
 */ 
 
 
 
