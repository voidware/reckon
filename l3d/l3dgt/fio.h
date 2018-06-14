/**
 * Copyright (c) 2009 Voidware Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS," WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
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


