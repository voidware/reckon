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

#include <stdio.h>
#include <stdarg.h>

// casio SDK headers
#include "filebios.h"
#include "fxlib.h"
#include "setjmp.h"
#include "../../utils.h"
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

typedef struct
{
    int x;
    int y;
} Pos;

static Pos pos;
static jmp_buf jmpExit;
static FDInfo FDInfoTable[MAX_FD];

int CasioInit()
{
    int v;
    pos.x = 1; pos.y = 1; locate(pos.x, pos.y);
    v = setjmp(jmpExit);
    return v;
}

void exit(int arg)
{
    // back to CasioInit
    longjmp(jmpExit, 1);
}

void scroll1()
{
    // scroll 1 line
    unsigned char* d = GetVRAMPtr();
    unsigned char* p = d + 128;  // 128 bytes per line
    memmove(d, p, 128*7); // move 7 lines
    memset(d + 128*7, 0, 128); // erase last line    

    pos.x = 1; 
    pos.y = 8;
    locate(pos.x, pos.y);
    Bdisp_PutDisp_DD();
}

void newline()
{
    pos.x = 1;
    if (++pos.y == 9)
        scroll1();
    else
        locate(pos.x, pos.y);
}


int _printc(char* c)
{
    int cc = 1;
    if (*c == '\b')
    {
        if (pos.x > 1) --pos.x;
        else if (pos.y > 1)
        {
            --pos.y;
            pos.x = 21;
        }
        else return cc;

        locate(pos.x, pos.y);
        *c = ' ';
        PrintC((unsigned char*)c);
        locate(pos.x, pos.y);
    }
    else if (*c == '\n')
        newline();
    else 
    {
        PrintC((unsigned char*)c);
        if (*c == '\\') ++cc;
        if (++pos.x == 22) newline();
            
    }
    return cc;
}

int printc(char* c)
{
    int s = _printc(c);
    Bdisp_PutDisp_DD();
    return s;
}

void print(char* s)
{
    while (*s)
        s += _printc(s);
    Bdisp_PutDisp_DD();
}

void printAt(const char* s, int x, int y)
{
    pos.x = x; 
    pos.y = y;
    locate(pos.x, pos.y);
    print(s);
}

int printf(const char* fmt, ...)
{
    // XX HACk
    char buf[128];

    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);

    print(buf);
}

int fprintf(FILE* st, const char* fmt, ...)
{
    // XX HACk
    char buf[128];

    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);

    print(buf);
}


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
