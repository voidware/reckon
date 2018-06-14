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

#ifndef __stream_h__
#define __stream_h__

struct Streamer
{
    virtual void        put(char) = 0;
    virtual void        unput() = 0;
    virtual void        put(const char*) = 0;
    virtual char*       finish() = 0;
    virtual int         get() = 0;
};

struct BufStreamer: public Streamer
{
    BufStreamer(char* buf, int bsize)
        : buf_(buf), size_(bsize) { pos_ = buf_; }

    void                put(char c)
    {
        if (pos_ - buf_ < size_-1) *pos_++ = c;
    }

    void                unput()
    {
        // erase a charater 
        if (pos_ > buf_) --pos_;
    }

    void                put(const char* s)
    {
        while (*s)
        {
            if (pos_ - buf_ < size_-1) *pos_++ = *s;
            ++s;
        }
    }

    char*               finish()
    {
        *pos_ = 0;
        pos_ = buf_;
        return buf_;
    }

    int                 get()
    {
        /* get character at last position */
        if (pos_ > buf_) return pos_[-1];
        return 0;
    }

    char*               buf_;
    int                 size_;
    char*               pos_;
};

#endif // __stream_h__
