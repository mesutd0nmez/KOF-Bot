/*

crc32.h

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1992 Tatu Ylonen, Espoo, Finland
All rights reserved

Created: Tue Feb 11 14:37:27 1992 ylo

Functions for computing 32-bit CRC.

*/

/*
* $Id: crc32.h,v 1.1.1.1 1996/02/18 21:38:11 ylo Exp $
* $Log: crc32.h,v $
* Revision 1.1.1.1  1996/02/18 21:38:11  ylo
* 	Imported ssh-1.2.13.
*
* Revision 1.2  1995/07/13  01:21:45  ylo
* 	Removed "Last modified" header.
* 	Added cvs log.
*
* $Endlog$
*/
#ifndef CRC32_H
#define CRC32_H

/* This computes a 32 bit CRC of the data in the buffer, and returns the
CRC.  The polynomial used is 0xedb88320. */

#include <cstdint>

uint32_t crc32(const uint8_t *s, uint32_t len, uint32_t startVal = 0);

#endif /* CRC32_H */

