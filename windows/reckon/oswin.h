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

#ifndef __oswin_h__
#define __oswin_h__

#define KEY_CTRL_LEFT 1
#define KEY_CTRL_RIGHT 2
#define KEY_CTRL_DOWN 3
#define KEY_CTRL_UP 4
#define KEY_CTRL_AC 0
#define KEY_CHAR_PLUS '+'
#define KEY_CHAR_MINUS '-'

inline int GetKeyNonblocking(int wait, int x)
{
    return 0;
}

inline void AC()
{
}

inline int EscapeKeyPressed()
{
    return 0;
}

inline void CPUSpeedFast() {}
inline void CPUSpeedNormal() {}

inline void Update3DScreen() {}

#endif 
