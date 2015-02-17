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

#include <stdio.h>
#include <stdarg.h>

// casio SDK headers
#include "filebios.h"
#include "fxlib.h"
#include "setjmp.h"
#include "utils.h"
#include "fio.h"

#define MAX_FNAME   20
#define MAX_FD    10

#define DIM(_x) (sizeof(_x)/sizeof((_x)[0]))

typedef struct
{
    int fd;
    int pos;
    int index;
} FDInfo;

static FDInfo FDInfoTable[MAX_FD];

static FDInfo* _findFD(int fd)
{
    int i;
    for (i = 0; i < MAX_FD; ++i)
    {
        FDInfo* fdi = FDInfoTable + i;
        if (fdi->fd == fd)
        {
            fdi->index = i;
            return fdi;
        }
    }
    return 0;
}

static void _fontName(const char* name, FONTCHARACTER* fname)
{
    static const FONTCHARACTER path[] = { '\\', '\\', 'f','l','s','0','\\' };
    int i, j;
    for (j = 0; j < DIM(path); ++j) fname[j] = path[j];
    for (i = 0; j < MAX_FNAME-1; ++i, ++j)
        if (!(fname[j] = name[i])) break;
    fname[j] = 0;
}


int open(const char* name, int mode, int createmode)
{
    FONTCHARACTER cName[MAX_FNAME];
    int fd;
    int bmode = _OPENMODE_READWRITE; // default
    
    if (mode == O_RDONLY) bmode = _OPENMODE_READ;
    if (mode == O_WRONLY) bmode = _OPENMODE_WRITE;
    
    _fontName(name, cName);
    fd = Bfile_OpenFile(cName, bmode);

    if (fd > 0)
    {
        // track the FD info
        FDInfo* fdi = _findFD(0); // find a spare.
        if (fdi)
        {
            fdi->fd = fd;
            fdi->pos = 0;
            fd = fdi->index + 3;
        }
        else
        {
            Bfile_CloseFile(fd);
            fd = -1;
        }
    }
    return fd;
}

int close(int fd)
{
    FDInfo* fdi;

    if (fd < 3) return 0; // ignore

    // clear Fdinfo
    fdi = FDInfoTable + (fd - 3);
    Bfile_CloseFile(fdi->fd);
    fdi->fd = 0; // release slot
    return 0;
}

int read(int fd, char* buf, int size)
{
    int v = -1;
    if (fd < 3)
    {
        // read stdin?
        v = 0;
    }
    else
    {
        FDInfo* fdi = FDInfoTable + (fd - 3);
        v = Bfile_ReadFile(fdi->fd, buf, size, -1);
        if (v > 0)
            fdi->pos += v; // update position
    }
    return v;
}

int lseek(int fd, long dist, int where)
{
    int v = -1;
    if (fd > 3)
    {
        FDInfo* fdi = FDInfoTable + (fd - 3);
        switch (where)
        {
        case SEEK_SET:
            if (Bfile_SeekFile(fdi->fd, dist) >= 0)
            {
                v = dist;
                fdi->pos = dist;
            }
            break;
        case SEEK_CUR:
            return lseek(fd, dist + fdi->pos, SEEK_SET);
            break;
        case SEEK_END:
            {
                int s = Bfile_GetFileSize(fdi->fd);
                return lseek(fd, dist + s, SEEK_SET);
            }
            break;
        }
    }
    return v;
}

int write(int fd, char* buf, int size)
{
    // NB: must write even number of bytes.
    int v = -1;
    if (fd < 3)
    {
        int i;
        if (!fd) return 0; // write stdin!
        
        // write to stdout or stderr
        
        for (i = 0; i < size; ++i) _printc(buf + i);
        Bdisp_PutDisp_DD();
        v = size;
    }
    else
    {
        FDInfo* fdi = FDInfoTable + (fd - 3);
        int p = Bfile_WriteFile(fdi->fd, buf, size);
        if (p >= 0)
        {
            v = p - fdi->pos; // bytes written
            fdi->pos = p;
        }
    }
    return v;
}

static FILE iob[MAX_FD];

int _fillbuf(FILE* fp)
{
    int v = EOF;
    int n = read(fp->_iofd, (char*)fp->_bufbase, BUFSIZ);
    fp->_bufptr = fp->_bufbase;
    fp->_buflen = n;
    fp->_bufcnt = n;
    if (n)
    {
        --fp->_bufcnt;
        v = (int)*fp->_bufptr++;
    }
    return v;
}

FILE* fopen(const char* name, const char* mode)
{
    FILE* fp = 0;
    int fd = open(name, O_RDONLY, 0);
    if (fd > 0)
    {
        fp = iob + (fd - 3);
        memset(fp, 0, sizeof(*fp));
        fp->_bufbase = (unsigned char*)xmalloc(BUFSIZ);
        fp->_ioflag1 = _IORW | _IOWRITE;
        fp->_iofd = fd;
    }
    return fp;
}

int fclose(FILE* fp)
{
    if (fp)
    {
        close(fp->_iofd);
    }
    return 0;
}
