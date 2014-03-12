/* 
 * 
 *	  Name: cmsfs.c (C source) 
 * 
 */ 



#include	<stdlib.h> 
#include	<time.h> 
#include	<unistd.h> 
#include	<stdio.h> 
#include	<fcntl.h> 

#include	"cmsfs.h" 
#include	"aecs.h" 

#include	<stdio.h> 

/* ------------------------------------------------------------ CMSFSBEX 
    convert a big-endian integer into a local integer 
 */ 
int cmsfsbex ( char *ivalue , int l ) 
  { 
    int 	i , ovalue ; 
    ovalue = ivalue[0] ; 
    for (i = 1 ; i < l ; i++) 
      { 
	ovalue = ovalue << 8 ; 
	ovalue += ivalue[i] & 0xFF; 
      } 
    return ovalue ; 
  } 
 
/* ------------------------------------------------------------ CMSFSX2I 
 *  CMS FS heXadecimal-to-Integer function 
 */ 
int cmsfsx2i ( char *ivalue , int l ) 
  { 
    int 	i , ovalue ; 
    ovalue = (ivalue[0] & 0x0F) + ((ivalue[0]>>4) & 0x0F) * 10; 
    for (i = 1 ; i < l ; i++) 
      { 
	ovalue = ovalue << 8 ; 
	ovalue += (ivalue[i] & 0x0F) + ((ivalue[i]>>4) & 0x0F) * 10; 
      } 
    return ovalue ; 
  } 
 
/* ------------------------------------------------------------ CMSFSMAP 
 *  Map a CMS FS FST structure to our own "inode" structure. 
 *  Operation is function(target,source). 
 */ 
int cmsfsmap(struct CMSINODE *finode , struct FSTD *cmsfst , int blocksize) 
  { 
    int 	i; 
    char       *p, *q; 
 
    /*  extract and translate filename  */ 
    p = cmsfst->FSTFNAME; 
    q = finode->name; 
    i = 0; 
    while (*p != 0x40 && *p != 0x00 && i < 8) 
	{ *q++ = chretoa(*p++);  i++; } 
    *q++ = '.'; 
    p = cmsfst->FSTFTYPE; 
    i = 0; 
    while (*p != 0x40 && *p != 0x00 && i < 8) 
	{ *q++ = chretoa(*p++);  i++; } 
    *q++ = 0x00; 
 
    /*  extract and translate RECFM  */ 
    p = cmsfst->FSTRECFM; 
    q = finode->recfm; 
    q[0] = chretoa(p[0]); 
    q[1] = 0x00; 
 
    /*  extract LRECL  */ 
    finode->lrecl = cmsfsbex(cmsfst->FSTLRECL,4); 
 
    /*  compute file offset from FOP  */ 
    finode->start = cmsfsfom(cmsfsbex(cmsfst->FSTFOP,4),blocksize); 
    finode->start = ( cmsfsbex(cmsfst->FSTFOP,4) - 1 ) * blocksize ; 
 
    /*  extract block count  */ 
    finode->bloks = cmsfsbex(cmsfst->FSTADBC,4); 
 
    /*  extract item count  */ 
    finode->items = cmsfsbex(cmsfst->FSTAIC,4); 
 
    /*  extract date and compute time stamp  */ 
    { 
	int year, mon, day, hh, mm, ss; 
    year = ((cmsfst->FSTADATI[0] & 0xF0) >> 4) * 10; 
    year += cmsfst->FSTADATI[0] & 0x0F; 
    if (cmsfst->FSTFLAGS[0] & FSTCNTRY) year += 2000; 
				else	year += 1900; 
    mon  = ((cmsfst->FSTADATI[1] & 0xF0) >> 4) * 10; 
    mon  += cmsfst->FSTADATI[1] & 0x0F; 
    day  = ((cmsfst->FSTADATI[2] & 0xF0) >> 4) * 10; 
    day  += cmsfst->FSTADATI[2] & 0x0F; 
    hh   = ((cmsfst->FSTADATI[3] & 0xF0) >> 4) * 10; 
    hh  += cmsfst->FSTADATI[3] & 0x0F; 
    mm   = ((cmsfst->FSTADATI[4] & 0xF0) >> 4) * 10; 
    mm  += cmsfst->FSTADATI[4] & 0x0F; 
    ss   = ((cmsfst->FSTADATI[5] & 0xF0) >> 4) * 10; 
    ss  += cmsfst->FSTADATI[5] & 0x0F; 
/* 
(void) printf("%04d-%02d-%02d %02d:%02d:%02d",year,mon,day,hh,mm,ss); 
 */ 
    } 
 
    finode->level = cmsfst->FSTNLVL[0] & 0xFF ; 
    finode->psize = cmsfst->FSTPTRSZ[0] & 0xFF ; 
 
/* 
		cmsfst.FSTDATEW[0],cmsfst.FSTDATEW[1],
		cmsfst.FSTTIMEW[0],cmsfst.FSTTIMEW[1], 
		cmsfst.FSTYEARW[0],cmsfst.FSTYEARW[1]); 
!! above may be all zeros; why? 
*/ 
 
/* ---- * 
(void) printf("FSTADATI %02X-%02X-%02X %02X:%02X:%02X ",
		cmsfst.FSTADATI[0],cmsfst.FSTADATI[1],
		cmsfst.FSTADATI[2],cmsfst.FSTADATI[3],
		cmsfst.FSTADATI[4],cmsfst.FSTADATI[5]); 
 * ---- */ 
 
/* 
nrecs = cmsfst.FSTRECCT[0]; 
nrecs = nrecs << 8 ; 
nrecs += cmsfst.FSTRECCT[1]; 
(void) printf("%d ",nrecs); 
!! above may be zero; why? 
 */ 
 
    return 0; 
  } 
 
/* ------------------------------------------------------------ CMSFSAL0 
 *  Map (or read) an entire file into storage with no indirection. 
THIS FUNCTION IS BEING REPLACED
 */ 
void *cmsfsal0(int fd , int origin , int blocks , int blksz ) 
  { 
	int bytes; 
	char *space; 
	int rc; 
 
	bytes = blocks * blksz; 
	space = malloc(bytes); 
	if (space == NULL) 
	  { 
	    (void) perror("malloc()"); 
	    (void) printf("malloc(%d) failed\n",bytes); 
	    return NULL; 
	  } 
 
	rc = lseek(fd,origin,SEEK_SET); 
	if (rc != origin) { (void) perror("lseek()"); 
	    (void) printf("read(,%d,) failed\n",origin); return NULL; } 
	rc = read(fd,space,bytes); 
	if (rc != bytes) { (void) perror("read() file") ; 
	    (void) printf("read(,,%d) failed\n",bytes); return NULL; } 
 
	return space; 
  } 
 
/* ------------------------------------------------------------ CMSFSTXT 
 *  File in storage,  treat it as text  (thus translate E to A 
 *  and insert line breaks),  and write to standard output. 
 */ 
int cmsfstxs(int fd1 , char *origin , int items , int lrecl ) 
  { 
    int 	i, j, k, l; 
    char	buffer[80]; 
 
    for (i = 0 ; i < items ; i++) 
      { 
	k = lrecl; 
	if (k == 0) 
	  { 
	    k = cmsfsbex((void*)origin,2); 
	    origin++ ; origin++ ; 
	  } 
if (k == 0) break; 
	l = k; 
	for (j = 0; j < 80 && j < l; j++) 
	  { 
	    buffer[j] = chretoa(origin[j]); 
	  } 
	buffer[j] = 0x00; 
	(void) printf("%s\n",buffer); 
	(void) fflush(stdout); 
	origin += k; 
      } 
  } 
 
/*									< */ 
/* ------------------------------------------------------------ CMSFSBKR 
 *  CMS FS Block Read 
 */ 
int cmsfsbkr(CMSINODE *inode,char *buffer,int blockno) 
  { 
    int 	rc, offset; 

    if (inode == NULL) return -1; 
    if (inode->bksiz == 0) return -1; 
    switch (inode->level) 
      { 
	case 0: offset = inode->start + (blockno - 1) * inode->bksiz; 
		rc = lseek(inode->devfd,offset,SEEK_SET); 
		if (rc != offset) return -1; 
		rc = read(inode->devfd,buffer,inode->bksiz); 
		if (rc != inode->bksiz) return -1; 
		return rc; 
/*									< */ 
	case 1: 
		rc = lseek(inode->devfd,inode->start,SEEK_SET); 
		if (rc != inode->start) return -1; 
		rc = read(inode->devfd,buffer,inode->bksiz); 
		if (rc != inode->bksiz) return -1; 
		offset = (blockno - 1) * inode->psize; 
		offset = (cmsfsbex(&buffer[offset],4) 
				  - 1) * inode->bksiz; 
		rc = lseek(inode->devfd,offset,SEEK_SET); 
		if (rc != offset) return -1; 
		rc = read(inode->devfd,buffer,inode->bksiz); 
		if (rc != inode->bksiz) return -1; 
		return rc; 

	default: 
(void) printf("cmsfsbkr(): indirection level %d not supported\n",inode->level); 
		return -1; 
      } 
    return 0; 
  } 


void cmsfsdmp(char *p,int l) 
  { 
    int 	i; 
    for (i = 0; i < l; i+=32) 
      { 
	(void) printf("%02X%02X%02X%02X ", 
p[i]&0xFF,p[i+1]&0xFF,p[i+2]&0xFF,p[i+3]&0xFF); 
	(void) printf("%02X%02X%02X%02X ", 
p[i+4]&0xFF,p[i+5]&0xFF,p[i+6]&0xFF,p[i+7]&0xFF); 
	(void) printf("%02X%02X%02X%02X ", 
p[i+8]&0xFF,p[i+9]&0xFF,p[i+10]&0xFF,p[i+11]&0xFF); 
	(void) printf("%02X%02X%02X%02X  ", 
p[i+12]&0xFF,p[i+13]&0xFF,p[i+14]&0xFF,p[i+15]&0xFF); 

	(void) printf("%02X%02X%02X%02X ", 
p[i+16]&0xFF,p[i+17]&0xFF,p[i+18]&0xFF,p[i+19]&0xFF); 
	(void) printf("%02X%02X%02X%02X ", 
p[i+20]&0xFF,p[i+21]&0xFF,p[i+22]&0xFF,p[i+23]&0xFF); 
	(void) printf("%02X%02X%02X%02X ", 
p[i+24]&0xFF,p[i+27]&0xFF,p[i+26]&0xFF,p[i+27]&0xFF); 
	(void) printf("%02X%02X%02X%02X\n", 
p[i+28]&0xFF,p[i+29]&0xFF,p[i+30]&0xFF,p[i+31]&0xFF); 
      } 
  } 
 


/**********************************************************************/ 
int cmsfsop1(char *dn,struct CMSVOLID *vol) 
  { 
    int 	i,	/*  a counter  */ 
		rc,	/*  a return code  */ 
		dd,	/*  DEV fd  */ 
		bktry;	/*  a trial blocksize  */ 
 
    int 	cmsfsbk3;  /*  DELETE ME  */ 
 
    struct	CMSINODE	rootinode; 
 
    /*  partial "ADT" structure (per IBM)  */ 
    struct	CMSFSADT	cmsvol; 
 
    /*  "FST" structure (per IBM)  */ 
    struct	FSTD		cmsfst; 
 
    /*  array of possible CMS FS blocksizes  */ 
    int 	cmsfsbkt[8]	=	CMSFSBKT ; 
 
    /*  char array (4 bytes) "CMS1" filesystem identifier  */ 
    char	cmsfsvsk[8]	=	CMSFSVSK ; 
    /*  (a "magic number" by any other name)  */ 
 
    /*  open the filesystem as an ordinary file  */ 
    dd = open(dn,O_RDONLY); 
    if (dd < 0) 
      { 
	(void) perror("open()"); 
	return dd; 
      } 
 
    /*  set this value to zero because it serves as a check later  */ 
    vol->blksz = 0; 
 
    /*  look for the CMS volume label (aka VOLSER)  */ 
    for (i = 0 ; cmsfsbkt[i] != 0 ; i++) 
      { 
	int	index; 
	bktry = cmsfsbkt[i]; 
(void) printf("trying %d blocksize ...\n",bktry); 
	index = bktry * 2; 
(void) printf("lseek(,%d,SEEK_SET)\n",index); 
	rc = lseek(dd,index,SEEK_SET); 
	if (rc != index) { (void) perror("lseek()"); break; } 
 
	rc = read(dd,&cmsvol,sizeof(cmsvol)); 
	if (rc != sizeof(cmsvol)) { (void) perror("read() VOL"); break; } 
 
	if (cmsvol.ADTIDENT[0] == cmsfsvsk[0] 
	 && cmsvol.ADTIDENT[1] == cmsfsvsk[1] 
	 && cmsvol.ADTIDENT[2] == cmsfsvsk[2] 
	 && cmsvol.ADTIDENT[3] == cmsfsvsk[3]) 
	  { 
	    int     blksz; 
	    blksz = cmsfsbex(cmsvol.ADTDBSIZ,4); 
	    if (blksz == bktry) 
	      { 
		vol->blksz = bktry ; 
		break; 
	      } 
    (void) printf("FS blocksize %d does not match disk blocksize %d\n", 
			blksz,bktry); 
	  } 
      } 
 
    /*  did we find a CMS1 volume label?  */ 
    if (vol->blksz == 0) 
      { 
	(void) printf("Can not find a CMS1 label on %s.\n",dn); 
	(void) close(dd); 
	return -1; 
      } 
 
    /*  extract volume label and translate  */ 
    for (i = 0 ; i < 6 ; i++) vol->volid[i] = cmsvol.ADTID[i] ; 
    vol->volid[6] = 0x00; 
    (void) stretoa(vol->volid); 
 
    /*  extract "disk origin pointer"  */ 
    vol->origin = cmsfsbex(cmsvol.ADTDOP,4); 
 
    /*  extract number of cylinders used  */ 
    vol->ncyls = cmsfsbex(cmsvol.ADTCYL,4); 
 
    /*  extract max number of cylinders  */ 
    vol->mcyls = cmsfsbex(cmsvol.ADTMCYL,4); 
 
    /*  extract "total blocks on disk" count  */ 
    vol->blocks = cmsfsbex(cmsvol.ADTNUM,4); 
 
    /*  compute "blocks used" count  */ 
    vol->bkused = cmsfsbex(cmsvol.ADTUSED,4); 
    vol->bkused += 1 ;	/*  why???  */ 
 
    /*  compute "blocks free" count  */ 
    vol->bkfree = vol->blocks - vol->bkused ; 
 
      { 
	struct tm temptime;	/*  temporary  */ 
	temptime.tm_sec = cmsfsx2i(&cmsvol.ADTDCRED[5],1); 
	temptime.tm_min = cmsfsx2i(&cmsvol.ADTDCRED[4],1); 
	temptime.tm_hour = cmsfsx2i(&cmsvol.ADTDCRED[3],1); 
	temptime.tm_mday = cmsfsx2i(&cmsvol.ADTDCRED[2],1); 
	temptime.tm_mon = cmsfsx2i(&cmsvol.ADTDCRED[1],1) - 1; 
	temptime.tm_year = cmsfsx2i(&cmsvol.ADTDCRED[0],1); 
	if (cmsvol.ADTFLGL[0] & ADTCNTRY) 
		temptime.tm_year += 100 ; 

/* 
    (void) printf("Created %02X-%02X-%02X %02X:%02X:%02X\n", 
	cmsvol.ADTDCRED[0],cmsvol.ADTDCRED[1], 
	cmsvol.ADTDCRED[2],cmsvol.ADTDCRED[3], 
	cmsvol.ADTDCRED[4],cmsvol.ADTDCRED[5]); 
    (void) printf("Created %04d-%02d-%02d %02d:%02d:%02d\n", 
	temptime.tm_year,temptime.tm_mon,temptime.tm_mday, 
	temptime.tm_hour,temptime.tm_min,temptime.tm_sec); 
 */ 
 
    vol->ctime = mktime(&temptime); 
      } 
 
    vol->resoff = cmsfsbex(cmsvol.ADTOFFST,4); 
 
    vol->fstsz = cmsfsbex(cmsvol.ADTFSTSZ,4); 
    vol->fstct = cmsfsbex(cmsvol.ADTNFST,4); 
 
 
    /*  origin is a "base 1" number;  convert to "base 0" offset  */ 
    cmsfsbk3 = cmsfsfom(vol->origin,bktry); 
    cmsfsbk3 = cmsfsfom(vol->origin,vol->blksz); 
 
    /*  read in the first FST  */ 
    rc = lseek(dd,cmsfsbk3,SEEK_SET); 
    if (rc != cmsfsbk3) { (void) perror("lseek()"); 
	(void) close(dd); 
return rc; } 
    rc = read(dd,&cmsfst,sizeof(cmsfst)); 
    if (rc != sizeof(cmsfst)) { (void) perror("read() FST") ; 
	(void) close(dd); 
return rc; } 
 
    /*  map the inode for the directory  */ 
    rc = cmsfsmap(&rootinode,&cmsfst,vol->blksz); 
    if (rc != 0)  { (void) perror("cmsfsmap()"); 
	(void) close(dd); 
return rc; } 
 
    /*  sanity check:  FOP in dir entry must match DOP in vol  */ 
    if (cmsfsbk3 != rootinode.start) 
      { 
	char	buff[32];	/*  temporary  */ 
	(void) printf("FOP %d does not match DOP %d BS %d\n", 
		rootinode.start,cmsfsbk3,vol->blksz); 
	(void) printf("blocksize %d levels %d\n", 
		vol->blksz,rootinode.level); 
	(void) lseek(dd,rootinode.start,SEEK_SET); 
	(void) read(dd,buff,32); 
	(void) cmsfsdmp(buff,32); 
	(void) lseek(dd,cmsfsbk3,SEEK_SET); 
	(void) read(dd,buff,32); 
	(void) cmsfsdmp(buff,32); 
      } 
 
    /*  sanity check:  RECFM of a directory must be "F"  */ 
    if (rootinode.recfm[0] != 'F') 
      { 
	(void) printf("directory RECFM %s not 'F'\n",rootinode.recfm); 
	(void) close(dd); 
	return -1; 
      } 
 
    /*  sanity check:  LRECL of a directory must be 64  */ 
    if (rootinode.lrecl != 64) 
      { 
	(void) printf("directory LRECL %d not 64\n",rootinode.lrecl); 
	(void) close(dd); 
	return -1; 
      } 
 
    vol->devfd = dd; 
    vol->files = rootinode.items; 
 
    /*  report  */ 
/* 
    (void) printf("Volume '%s', blksz %d, files %d, directory blocks %d\n", 
		vol->volid,vol->blksz,vol->files,rootinode.bloks); 
 */ 
    (void) printf("Volume '%s' blksz %d files %d\n", 
		vol->volid,vol->blksz,vol->files); 
 
    return 0; 
  } 
 

