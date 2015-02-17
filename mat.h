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

#ifndef __mat_h__
#define __mat_h__

#include "types.h"

typedef BCD VALUE;
typedef TermRef Mat;
typedef TermRef Vec;

#define MAT(_m,_n,_r,_c) FLOAT(ARRAY(ARRAY(_m)->_at(_r))->_at(_c))->v_.asBCD()
#define VEC(_m,_i) FLOAT(ARRAY(_m)->_at(_i))->v_.asBCD()
#define VECC(_m,_i) COMPLEX(ARRAY(_m)->_at(_i))->cmf_

#define VMAT(_m,_nr,_r,_c) \
((_nr == 1) ? VEC(_m, _c) : MAT(_m,0,_r,_c)) 


/* because we hate templates, the string VALUE, represents the
 * templated type.
 */
struct Matrix
{
    // Constructors
    
    Matrix() {}
    Matrix(Term* t) : _a(t) { _init(); }

    void clone(const Matrix& a)
    {
        _a = ARRAY(a._a)->clone();
        _init();
    }

    friend bool mul(const Matrix& a, const Matrix& b, Matrix& c);
    friend bool add(const Matrix& a, const Matrix& b, Matrix& c);
    friend bool sub(const Matrix& a, const Matrix& b, Matrix& c);
    friend bool div(const Matrix& a, const Matrix& b, Matrix& c);
    friend void transpose(const Matrix& a, Matrix& c);
    friend bool invert(const Matrix& b, Matrix& c, bool& singular);
    static void _create(Matrix& a, int nRows, int nCols, bool complex = false);

    bool determinant(VALUE&) const;
    bool isSquare() const { return _nRows == _nCols; }

    void _init()
    {
        _nRows = 0;
        _nCols = 0;
        int f = ARRAY(_a)->flags_;
        if (f == Array::array_realvector)
        {
            _nCols = ARRAY(_a)->size_;
            _nRows = 1;
        }
        else if (f == Array::array_realmatrix)
        {
            _nRows = ARRAY(_a)->size_;
            _nCols = ARRAY(_a)->nCols_;
        }
    }

    unsigned int                _nRows;
    unsigned int                _nCols;
    TermRef                     _a;     // will be an array
};

bool prootEigen(const Matrix& a, Matrix& rmat);

#endif // __mat_h__
