/* ----------------------------------------------------------------- ÆCS
 * ASCII to EBCDIC and vice-versa code conversion tables.
 * Tables included here are based on ASCII conforming to the ISO8859-1
 * Latin 1 character set and EBCDIC conforming to the IBM Code Page 37
 * Latin 1 character set (except for three pairs of characters in 037).
 * This header file is included by AECS.C and others.
 */

#ifndef AECS_HEADER
#define AECS_HEADER

extern int chratoe(unsigned int c);
#define chratoem(c) ebc8859[(int)c]
extern int stratoe(unsigned char *s);

extern int chretoa(unsigned int c);
#define chretoam(c) asc8859[(int)c]
extern int stretoa(unsigned char *s);

#endif


