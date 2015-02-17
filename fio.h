/**
 *
 * Copyright (c) 2010-2015 Voidware Ltd.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code as
 * defined in and that are subject to the Voidware Public Source Licence version
 * 1.0 (the 'Licence'). You may not use this file except in compliance with the
 * Licence or with expressly written permission from Voidware.  Please obtain a
 * copy of the Licence at http://www.voidware.com/legal/vpsl1.txt and read it
 * before using this file.
 * 
 * The Original Code and all software distributed under the Licence are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS
 * OR IMPLIED, AND VOIDWARE HEREBY DISCLAIMS ALL SUCH WARRANTIES, INCLUDING
 * WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 *
 * Please see the Licence for the specific language governing rights and 
 * limitations under the Licence.
 *
 * contact@voidware.com
 */

#ifndef __fio_h__
#define __fio_h__

#include <stdio.h>

/* The casio headers declare fopen etc. and `FILE' but do not implement
 * the methods!
 *
 * here we implement them on top of simple `open' methods, which
 * in turn are implemented in terms of the casio file library.
 * 
 * casio's FILE structure looks like this:

 extern struct _iobuf {
       unsigned char *_bufptr;          // buffer pointer              
                long  _bufcnt;          // buffer count                
       unsigned char *_bufbase;         // buffer base pointer         
                long  _buflen;          // buffer length               
                char  _ioflag1;         // I/O control flag 1          
                char  _ioflag2;         // I/O control flag 2          
                char  _iofd;
  }  _iob[_NFILE];

  * 
  * and the declarations look like:
  *
  * extern FILE   *fopen(const char *, const char *);
  * extern int     fclose(FILE *);
  *
  */


#ifndef SEEK_SET
#define SEEK_SET 0
#endif

#ifndef SEEK_CUR
#define SEEK_CUR  1
#endif

#ifndef SEEK_END
#define SEEK_END   2
#endif

#ifndef O_RDONLY
#define O_RDONLY  0x0000
#endif

#ifndef O_BINARY
#define O_BINARY 0x8000
#endif

#ifndef O_WRONLY
#define O_WRONLY 1
#endif

#ifndef O_APPEND
#define O_APPEND 8
#endif

#ifndef O_CREAT
#define O_CREAT 0x100
#endif

#ifndef O_TRUNC
#define O_TRUNC 0x200
#endif

#ifndef O_EXCL
#define O_EXCL 0x400
#endif

extern int CasioInit();

extern int open(const char* name, int mode, int createmode);
extern int close(int fd);
extern int read(int fd, char* buf, int size);
extern int lseek(int fd, long dist, int where);
extern int write(int fd, char* buf, int size);

#endif 


