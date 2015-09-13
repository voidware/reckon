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

#include "mat.h"
#include "bcdmath.h"

using namespace bcdmath;


#ifdef _WIN32xx
#include <stdio.h>      // debugging
void printMat(const Mat& a, int n)
{
    int i, j;
    for (i = 0; i < n; ++i)
    {
        for (j = 0; j < n-1; ++j) 
        {
            print(MAT(a,n,i,j));
            printf(" ");
        }
        print(MAT(a,n,i,j));
        printf("\n");
    }
    printf("\n");
}
#endif

static int luDecomp(Mat& a, unsigned int n, int* indx, int*);
extern void luSolve(const Mat& a, unsigned int n, int* indx,
                    const Mat& b, Mat& x, int col);
static VALUE luDet(const Mat& a, unsigned int n, int d);
static void eigenVectors(Mat a, unsigned int n, VALUE * d);
static void swapRows(Mat a, unsigned int n, unsigned int i, unsigned int j);
static void svd(Mat a, unsigned int m, unsigned int n, VALUE * w, Mat v);
static void matMul(const Mat& a, const Mat& b,
                   unsigned int m, unsigned int n, unsigned int l, Mat& c);
static void svdSub(Mat u, VALUE * w, Mat v, unsigned int m, unsigned int n,
                   VALUE * b, VALUE * x);
static void balance(Mat& a, int n);
static bool hqr(const Mat& a, int n, Mat& z);
static void matAdd(const Mat& a, const Mat& b,
                   unsigned int m, unsigned int n, Mat& c);
static void matSub(const Mat& a, const Mat& b,
                   unsigned int m, unsigned int n, Mat& c);


VALUE SIGN(const VALUE& a, const VALUE& b)
{
    VALUE t = fabs(a);
    if (b < 0) t = -t;
    return t;
}


/** Matrix, methods **************************************************/

#if 0
void Matrix::svd(Matrix& w, Matrix& v)
{
    w.reinit(cols_,1);
    v.reinit(cols_,cols_);
    ::svd(m_, rows_, cols_, w.m_, v.m_);
}

void Matrix::svdSolve(const Matrix& w, const Matrix& v, 
                      const Matrix& b, Matrix& x)
{
    x.reinit(cols_, 1);
    svdSub(m_, w.m_, v.m_, rows_, cols_, b.m_, x.m_);
}
#endif

bool mul(const Matrix& a, const Matrix& b, Matrix& c)
{
    if (a._nCols == b._nRows)
    {
        Matrix::_create(c, a._nRows, b._nCols);
        matMul(a._a, b._a, a._nRows, a._nCols, b._nCols, c._a);
        return true;
    }
    return false;
}

bool add(const Matrix& a, const Matrix& b, Matrix& c)
{
    if (a._nCols == b._nCols && a._nRows == b._nRows)
    {
        Matrix::_create(c, a._nRows, a._nCols);
        matAdd(a._a, b._a, a._nRows, a._nCols, c._a);
        return true;
    }
    return false;
}

bool sub(const Matrix& a, const Matrix& b, Matrix& c)
{
    if (a._nCols == b._nCols && a._nRows == b._nRows)
    {
        Matrix::_create(c, a._nRows, a._nCols);
        matSub(a._a, b._a, a._nRows, a._nCols, c._a);
        return true;
    }
    return false;
}

bool div(const Matrix& a, const Matrix& b, Matrix& c)
{
    bool ok = b.isSquare() && a._nRows == b._nRows;
    if (ok)
    {
        Matrix lub;
        lub.clone(b);
        
        int n = lub._nRows;
        int cols = a._nCols;
        int* indx = new int[n];
        int d;
        ok = luDecomp(lub._a, n, indx, &d);
        if (ok)
        {
            Matrix::_create(c, n, cols);
            for (int j = 0; j < cols; ++j)
            {
                luSolve(lub._a, n, indx, a._a, c._a, j); 
            }
        }
        delete [] indx;
    }
    return ok;
}

bool invert(const Matrix& b, Matrix& c, bool& singular)
{
    singular = false;
    bool ok = b.isSquare();
    if (ok)
    {
        if (b._nRows == 1)
        {
            // 1x1 
            const BCD& t = VEC(b._a,0);
            if (t == 0) 
            {
                singular = true;
                ok = false;
            }
            else
            {
                Matrix::_create(c, 1,1);
                VEC(c._a,0) = 1/t;
            }
            return ok;
        }

        Matrix lub;
        lub.clone(b);
        
        int n = lub._nRows;
        int* indx = new int[n];
        int d;

        ok = luDecomp(lub._a, n, indx, &d);
        if (ok)
        {
            Matrix::_create(c, n, n);
            for (int j = 0; j < n; ++j)
            {
                for (int i = 0; i < n; ++i)
                    MAT(c._a, 0, i, j) = (i == j);
            
                luSolve(lub._a, n, indx, c._a, c._a, j); 
            }
        }
        else
            singular = true;

        delete [] indx;
    }
    return ok;
}

bool Matrix::determinant(VALUE& det) const
{
    bool ok = isSquare();
    if (ok)
    {
        Matrix lub;
        lub.clone(*this);
        
        int n = lub._nRows;
        int d;
        if (luDecomp(lub._a, n, 0, &d))
            det = luDet(lub._a, _nCols, d);
        else 
            det = 0; // is singular
    }
    return ok;
}

void transpose(const Matrix& a, Matrix& c)
{
    Matrix::_create(c, a._nCols, a._nRows);
    unsigned int i, j;
    for (i = 0; i < a._nRows; ++i) 
        for (j = 0; j < a._nCols; ++j)
            VMAT(c._a, c._nRows, j, i) = VMAT(a._a, a._nRows, i, j);
}

void Matrix::_create(Matrix& a, int nRows, int nCols, bool complex)
{
    Array* aa;

    a._nRows = nRows;
    a._nCols = nCols;

    if (nRows == 1)
    {
        // create a vector
        aa = Array::create(nCols);

        // fill it with values
        for (int j = 0; j < nCols; ++j)
        {
            if (complex)
                aa->_at(j) = ComplexN::create();
            else
                aa->_at(j) = Float::create();
        }
    }
    else
    {
        aa = Array::create(nRows);
        for (int i = 0; i < nRows; ++i)
        {
            Array* r = Array::create(nCols);
            
            // fill it with values
            for (int j = 0; j < nCols; ++j)
            {
                if (complex)
                    r->_at(j) = ComplexN::create();
                else 
                    r->_at(j) = Float::create();
            }
            r->_initFlags();
            aa->_at(i) = r;
        }
    }
    aa->_initFlags();
    a._a = aa;
}

/** General code ****************************************************/

int luDecomp(Mat& a, unsigned int n, int* indx, int* d)
{
    /* LU Decomposition.
     *
     * a(n,n) is decomposed (overwritten)
     * and `indx' records the permutations and `d' is +/-1 depending
     * on even or odd row interchanges.
     *
     * Return 1 if ok.
     */

    int i, imax, j, k;
    VALUE  big, sum, t;
    VALUE * vv;
    
    vv = new VALUE[n];
    *d = 1;
    for (i = 0; i < n; ++i) 
    {
        big = 0;
        for (j = 0; j < n; ++j) 
        {
            t = fabs(MAT(a,n,i,j));
            if (t > big) big = t;
        }
        if (big == 0) 
        {
            // printf("lud, singular Matrix\n");
            delete [] vv;
            return 0; // fail
        }
        vv[i] = VALUE(1)/big;
    }

    for (j = 0; j < n; ++j) 
    {
        for (i = 0; i < j; ++i) 
        {
            sum = MAT(a, n, i, j);
            for (k = 0; k < i; ++k) sum -= MAT(a,n,i,k) * MAT(a,n,k,j);
            MAT(a,n,i,j) = sum;
        }
        big = 0;
        for (i = j; i < n; ++i) 
        {
            sum = MAT(a,n,i,j);
            for (k = 0; k < j; ++k) sum -= MAT(a,n,i,k) * MAT(a,n,k,j);
            MAT(a,n,i,j) = sum;
            t = vv[i] * fabs(sum);
            if (t >= big) 
            {
                big = t;
                imax = i;
            }
        }
        if (j != imax) 
        {
            for (k = 0; k < n; ++k) 
            {
                t = MAT(a,n,imax,k);
                MAT(a,n,imax,k) = MAT(a,n,j,k);
                MAT(a,n,j,k) = t;
            }
            *d = -(*d);
            vv[imax] = vv[j];
        }

        if (indx) indx[j] = imax;
        if (MAT(a,n,j,j) == 0)
        {
            // singular matrix. we can either perturb it and 
            // continue - or bail.
            // right now, we bail since we should be using SVD in any case.
            //MAT(a,n,j,j) = TINY;

            delete [] vv;
            return 0;
        }
        if (j != n-1) 
        {
            t = VALUE(1)/MAT(a,n,j,j);
            for (i = j+1; i < n; ++i) MAT(a,n,i,j) *= t;
        }
    }

    delete [] vv;
    return 1;
}

void luSolve(const Mat& a, unsigned int n, int* indx,
             const Mat& b, Mat& x, int col)
{
    /* Backsubstitute LU decomposition to solve,
     * A(n,n) * X(n,n) = B(n)
     *
     * X replaces b and A is an LU decomposition. `indx' is required
     * from the decomposition.
     */

    int i, ii, ip, j;
    VALUE  sum;

    ii = -1;
    for (i = 0; i < n; ++i) 
        MAT(x,0,i,col) = MAT(b,0,i,col);

    for (i = 0; i < n; ++i) 
    {
        ip = indx[i];
        sum = MAT(x,0,ip,col);
        MAT(x,0,ip,col) = MAT(x,0,i,col);
        if (ii >= 0) 
            for (j = ii; j <= i-1; ++j) sum -= MAT(a,n,i,j) * MAT(x,0,j,col);
        else if (sum != 0) ii = i;
        MAT(x,0,i,col) = sum;
    }
    for (i = n-1; i >= 0; --i) 
    {
        sum = MAT(x,0,i,col);
        for (j = i+1; j < n; ++j) sum -= MAT(a,n,i,j) * MAT(x,0,j,col);
        MAT(x,0,i,col) = sum/MAT(a,n,i,i);
    }
}

#if 0
static void householderReduction(Mat a, int n, VALUE * d, VALUE * e)
{
    int l, k, j, i;
    VALUE  scale, hh, h, g, f;

    for (i = n-1; i > 0; --i) 
    {
        l = i-1;
        h = scale = 0;
        if (l > 0) {
            for (k = 0; k <= l; ++k) scale += fabs(MAT(a,n,i,k));
            if (scale == 0) e[i] = MAT(a,n,i,l);
            else 
            {
                for (k = 0; k <= l; ++k) 
                {
                    MAT(a,n,i,k) /= scale;
                    h += MAT(a,n,i,k)*MAT(a,n,i,k);
                }
                f = MAT(a,n,i,l);
                if (f >= 0) g = -sqrt(h);
                else g = sqrt(h);
                e[i] = scale*g;
                h -= f*g;
                MAT(a,n,i,l) = f-g;
                f = 0;
                for (j = 0; j <= l; ++j) 
                {
                    MAT(a,n,j,i) = MAT(a,n,i,j)/h;
                    g = 0;
                    for (k = 0; k <= j; ++k) g += MAT(a,n,j,k)*MAT(a,n,i,k);
                    for (k = j+1; k <= l; ++k) g += MAT(a,n,k,j)*MAT(a,n,i,k);
                    e[j] = g/h;
                    f += e[j] * MAT(a,n,i,j);
                }
                hh = f/(h+h);
                for (j = 0; j <= l; ++j) 
                {
                    f = MAT(a,n,i,j);
                    g = e[j] - hh*f;
                    e[j] = g;
                    for (k = 0; k <= j; ++k) 
                        MAT(a,n,j,k) -= f*e[k] + g*MAT(a,n,i,k);
                }
            }
        }
        else
            e[i] = MAT(a,n,i,l);
        
        d[i] = h;
    }

    d[0] = 0;
    e[0] = 0;
    for (i = 0; i < n; ++i) 
    {
        if (d[i] != 0) 
        {
            for (j = 0; j < i; ++j) 
            {
                g = 0;
                for (k = 0; k < i; ++k) g += MAT(a,n,i,k)*MAT(a,n,k,j);
                for (k = 0; k < i; ++k) MAT(a,n,k,j) -= g*MAT(a,n,k,i);
            }
        }
        d[i] = MAT(a,n,i,i);
        MAT(a,n,i,i) = VALUE(1);
        for (j = 0; j < i; ++j) 
        {
            MAT(a,n,j,i) = 0;
            MAT(a,n,i,j) = 0;
        }
    }
}

static void tridiagQL(VALUE* d, VALUE* e, int n, Mat a)
{
    int m, l, iter, i, k;
    VALUE  s, r, p, g, f, dd, c, b;

    for (i=1; i < n; ++i) e[i-1] = e[i];
    e[n-1] = 0;
    
    for (l = 0; l < n; ++l) 
    {
        iter = 0;
        do 
        {
            for (m = l; m < n-1; ++m) 
            {
                dd = fabs(d[m]) + fabs(d[m+1]);
                if (fabs(e[m]) + dd == dd) break; /* ?? */
            }
            if (m != l) 
            {
                if (++iter == 30) 
                {
                    // printf("too many iterations\n");
                    return;
                }

                g = (d[l+1]-d[l])/(VALUE(2)*e[l]);
                r = hypot(g, VALUE(1));
                g = d[m] - d[l] + e[l]/(g+SIGN(r,g));
                s = c = VALUE(1);
                p = 0;
                for (i = m-1; i >= l; --i) 
                {
                    f = s*e[i];
                    b = c*e[i];
                    r = hypot(f, g);
                    e[i+1] = r;
                    if (r == 0) 
                    {
                        d[i+1] -= p;
                        e[m] = 0;
                        break;
                    }
                    s = f/r;
                    c = g/r;
                    g = d[i+1]-p;
                    r = (d[i]-g)*s + VALUE(2)*c*b;
                    p = s*r;
                    d[i+1] = g+p;
                    g = c*r-b;
                    for (k = 0; k<n; ++k) 
                    {
                        f = MAT(a,n,k,i+1);
                        MAT(a,n,k,i+1) = s*MAT(a,n,k,i) + c*f;
                        MAT(a,n,k,i) = c*MAT(a,n,k,i) - s*f;
                    }
                }
                if (r == 0 && i >= l) continue;
                d[l] -= p;
                e[l] = g;
                e[m] = VALUE(0);
            }
        } while (m != l);
    }
}

void eigenVectors(Mat a, unsigned int n, VALUE * d)
{
    /* `a' is symmetric */

    VALUE* e = new VALUE[n];

    /* first perform reduction to tridiagonal form */
    householderReduction(a, n, d, e);
    tridiagQL(d, e, n, a);
    delete [] e;
}

void swapRows(Mat a, unsigned int n, unsigned int i, unsigned int j)
{
    unsigned int k;
    for (k = 0; k < n; ++k) 
    {
        VALUE  t = MAT(a,n,i,k);
        MAT(a,n,i,k) = MAT(a,n,j,k);
        MAT(a,n,j,k) = t;
    }
}

void svd(Mat a, unsigned int m, unsigned int n, VALUE* w, Mat v)
{
    /* Singular Value Decomposition.
     *
     * a(m,n) -> u(m,n) * w(n) * v(n,n)^T 
     * where a is replaced by u.
     *
     * u is orthogonal, w is a diagonal and v is orthogonal.
     */
    int flag, i, its, j, k, nm;
    VALUE  anorm, c, f, g, h, s, scale, x, y, z;
    VALUE* rv;
    VALUE  t;

    rv = new VALUE[n];
    g = scale = anorm = 0;
    for (i = 0; i < n; ++i) 
    {
        rv[i] = scale*g;
        g = s = scale = 0;
        if (i < m) 
        {
            for (k = i; k < m; ++k) scale += fabs(MAT(a,n,k,i));
            if (scale != 0) 
            {
                for (k = i; k < m; ++k) 
                {
                    MAT(a,n,k,i) /= scale;
                    s += MAT(a,n,k,i)*MAT(a,n,k,i);
                }
                f = MAT(a,n,i,i);
                g = -SIGN(sqrt(s), f);
                h = f*g-s;
                MAT(a,n,i,i) = f-g;
                for (j = i+1; j < n; ++j) 
                {
                    s = 0;
                    for (k = i; k < m; ++k) s += MAT(a,n,k,i)*MAT(a,n,k,j);
                    f = s/h;
                    for (k = i; k < m; ++k) MAT(a,n,k,j) += f*MAT(a,n,k,i);
                }
                for (k = i; k < m; ++k) MAT(a,n,k,i) *= scale;
            }
        }
        w[i] = scale*g;
        g = s = scale = 0;
        if (i < m && i != n-1) 
        {
            for (k = i+1; k < n; ++k) scale += fabs(MAT(a,n,i,k));
            if (scale != 0) 
            {
                for (k = i+1; k < n; ++k) 
                {
                    MAT(a,n,i,k) /= scale;
                    s += MAT(a,n,i,k)*MAT(a,n,i,k);
                }
                f = MAT(a,n,i,i+1);
                g = -SIGN(sqrt(s), f);
                h = f*g-s;
                MAT(a,n,i,i+1) = f-g;
                for (k = i+1; k < n; ++k) rv[k] = MAT(a,n,i,k)/h;
                for (j = i+1; j < m; ++j) 
                {
                    s = 0;
                    for (k = i+1; k < n; ++k) s += MAT(a,n,j,k)*MAT(a,n,i,k);
                    for (k = i+1; k < n; ++k) MAT(a,n,j,k) += s*rv[k];
                }
                for (k = i+1; k < n; ++k) MAT(a,n,i,k) *= scale;
            }
        }
        t = fabs(w[i]) + fabs(rv[i]);
        if (t > anorm) anorm = t;
    }
    for (i = n-1; i >= 0; --i) 
    {
        if (i < n-1) 
        {
            if (g != 0) 
            {
                for (j = i+1; j < n; ++j)
                    MAT(v,n,j,i) = (MAT(a,n,i,j)/MAT(a,n,i,i+1))/g;
                for (j = i+1; j < n; ++j) 
                {
                    s = 0;
                    for (k = i+1; k < n; ++k) s += MAT(a,n,i,k)*MAT(v,n,k,j);
                    for (k = i+1; k < n; ++k) MAT(v,n,k,j) += s*MAT(v,n,k,i);
                }
            }
            for (j = i+1; j < n; ++j) 
            {
                MAT(v,n,i,j) = 0;
                MAT(v,n,j,i) = 0;
            }
        }
        MAT(v,n,i,i) = VALUE(1);
        g = rv[i];
    }
    nm = m;
    if (n < nm) nm = n;
    for (i = nm-1; i >= 0; --i) 
    {
        g = w[i];
        for (j = i+1; j < n; ++j) MAT(a,n,i,j) = 0;
        if (g != 0) 
        {
            g = VALUE(1)/g;
            for (j = i+1; j < n; ++j) 
            {
                s = 0;
                for (k = i+1; k < m; ++k) s += MAT(a,n,k,i)*MAT(a,n,k,j);
                f = (s/MAT(a,n,i,i))*g;
                for (k = i; k < m; ++k) MAT(a,n,k,j) += f*MAT(a,n,k,i);
            }
            for (j = i; j < m; ++j) MAT(a,n,j,i) *= g;
        }
        else for (j = i; j < m; ++j) MAT(a,n,j,i) = 0;
        MAT(a,n,i,i) += VALUE(1);
    }
    for (k = n-1; k >= 0; --k) 
    {
        its = 0;
        for (;;) 
        {
            int l;
            if (++its == 30) {
                // printf("svd, no convergence after 30 iterations\n");
                delete [] rv;
                return;
            }

            flag = 1;
            for (l = k; l >= 0; --l) 
            {
                nm = l-1;
                if ((fabs(rv[l]) + anorm) == anorm) 
                {
                    flag = 0;
                    break;
                }
                // if (nm < 0) printf("eek!\n");
                if ((fabs(w[nm]) + anorm) == anorm) break;
            }
            if (flag) 
            {
                c = 0;
                s = VALUE(1);
                for (i = l; i <= k; ++i) 
                {
                    f = s*rv[i];
                    rv[i] *= c;
                    if ((fabs(f) + anorm) == anorm) break;
                    g = w[i];
                    h = hypot(f, g);
                    w[i] = h;
                    h = VALUE(1)/h;
                    c = g*h;
                    s = -f*h;
                    for (j = 0; j < m; ++j) 
                    {
                        y = MAT(a,n,j,nm);
                        z = MAT(a,n,j,i);
                        MAT(a,n,j,nm) = y*c+z*s;
                        MAT(a,n,j,i) = z*c - y*s;
                    }
                }
            }
            z = w[k];
            if (l == k) 
            {
                if (z < 0) 
                {
                    w[k] = -z;
                    for (j = 0; j < n; ++j) MAT(v,n,j,k) = -MAT(v,n,j,k);
                }
                break;
            }
            x = w[l];
            nm = k-1;
            // if (nm < 0) printf("erk!\n");

            y = w[nm];
            g = rv[nm];
            h = rv[k];
            f = ((y-z)*(y+z)+(g-h)*(g+h))/(VALUE(2)*h*y);
            g = hypot(f, VALUE(1));
            f = ((x-z)*(x+z)+h*((y/(f+SIGN(g,f)))-h))/x;
            c = s = VALUE(1);
            for (j = l; j <= nm; ++j) 
            {
                int jj;

                i = j+1;
                g = rv[i];
                y = w[i];
                h = s*g;
                g = c*g;
                z = hypot(f,h);
                rv[j] = z;
                c = f/z;
                s = h/z;
                f = x*c + g*s;
                g = g*c - x*s;
                h = y*s;
                y *= c;
                for (jj = 0; jj < n; ++jj) 
                {
                    x = MAT(v,n,jj,j);
                    z = MAT(v,n,jj,i);
                    MAT(v,n,jj,j) = x*c + z*s;
                    MAT(v,n,jj,i) = z*c - x*s;
                }
                z = hypot(f,h);
                w[j] = z;
                if (z != 0) 
                {
                    z = VALUE(1)/z;
                    c = f*z;
                    s = h*z;
                }
                f = c*g + s*y;
                x = c*y - s*g;
                for (jj = 0; jj < m; ++jj) 
                {
                    y = MAT(a,n,jj,j);
                    z = MAT(a,n,jj,i);
                    MAT(a,n,jj,j) = y*c + z*s;
                    MAT(a,n,jj,i) = z*c - y*s;
                }
            }
            rv[l] = 0;
            rv[k] = f;
            w[k] = x;
        }
    }
    delete [] rv;
}
#endif

VALUE luDet(const Mat& a, unsigned int n, int d)
{
    /* calculate the determinant using the LU decomposition.
     * `a' is the LUD, `d' is the sign changes, returned
     * from luDecomp.
     */
    VALUE det(d);
    for (unsigned int i = 0; i < n; ++i) det *= MAT(a,n,i,i);
    return det;
}

void matAdd(const Mat& a, const Mat& b, 
            unsigned int m, unsigned int n, Mat& c)
{
    /* a(m,n) + b(m,n) -> c(m,n) */
    int i, j;

    if (m == 1) // 1 row
        // vector add
        for (i = 0; i < n; ++i) VEC(c,i) = VEC(a,i) + VEC(b,i);
    else
    {
        for (i = 0; i < m; ++i) 
            for (j = 0; j < n; ++j) 
                MAT(c,n,i,j) = MAT(a,n,i,j)+MAT(b,n,i,j);
    }
}

void matSub(const Mat& a, const Mat& b,
            unsigned int m, unsigned int n, Mat& c)
{
    /* a(m,n) - b(m,n) -> c(m,n) */
    int i, j;

    if (m == 1) // 1 row
        // vector add
        for (i = 0; i < n; ++i) VEC(c,i) = VEC(a,i) - VEC(b,i);
    else
    {
        for (i = 0; i < m; ++i) 
            for (j = 0; j < n; ++j) 
                MAT(c,n,i,j) = MAT(a,n,i,j)-MAT(b,n,i,j);
    }
}

void matMul(const Mat& a, const Mat& b,
            unsigned int m, unsigned int n, unsigned int l, Mat& c)
{
    /* a(m,n) * b(n,l) -> c(m,l) */

    unsigned int i, j, k;
    VALUE t;

    for (i = 0; i < m; ++i) 
        for (j = 0; j < l; ++j) 
        {
            t = 0;
            for (k = 0; k < n; ++k) t += VMAT(a,m,i,k)*VMAT(b,n,k,j);
            VMAT(c,m,i,j) = t;
        }
}

#if 0
void svdSub(Mat u, VALUE * w, Mat v, unsigned int m, unsigned int n,
            VALUE * b, VALUE * x)
{
    int k, j, i;
    VALUE  s;
    VALUE * tv;

    tv = new VALUE[n];
    for (j = 0; j < n; ++j) 
    {
        s = 0;
        if (w[j] != 0) 
        {
            for (i = 0; i < m; ++i) s += MAT(u,n,i,j)*b[i];
            s /= w[j];
        }
        tv[j] = s;
    }

    for (j = 0; j < n; ++j) 
    {
        s = 0;
        for (k = 0; k < n; ++k) s += MAT(v,n,j,k) * tv[k];
        x[j] = s;
    }
    delete [] tv;
}
#endif

/* routines for eigenvalues of non-symmetric matrices */

#define RADIX VALUE(10)

void balance(Mat& a, int n)
{
    /* balance the square Matrix `a' of nxn elements. */

    VALUE  sqr;
    VALUE  r, c, g, f, s;
    int last;
    int i, j;

    sqr = RADIX*RADIX;
    last = 0;
    while (!last) 
    {
        last = 1;
        for (i = 0; i < n; ++i) 
        {
            r = 0;
            c = 0;
            for (j = 0; j < n; ++j) 
            {
                if (i != j) 
                {
                    c += fabs(MAT(a,n,j,i));
                    r += fabs(MAT(a,n,i,j));
                }
            }

            if (c != 0 && r != 0) 
            {
                g = r/RADIX;
                f = 1;
                s = c+r;
                while (c < g) { f *= RADIX; c *= sqr; }
                g = r*RADIX;
                while (c > g) { f /= RADIX; c /= sqr; }
                if ((c+r)/f < VALUE(95)/VALUE(100)*s) 
                {
                    last = 0;
                    g = VALUE(1)/f;
                    for (j = 0; j < n; ++j) MAT(a,n,i,j) *= g;
                    for (j = 0; j < n; ++j) MAT(a,n,j,i) *= f;
                }
            }
        }
    }
}


void squelchMat(const Mat& a, int n, const VALUE& eps)
{
    int i, j;
    for (i = 0; i < n; ++i)
    {
        for (j = 0; j < n; ++j)
            if (fabs(MAT(a,n,i,j)) <= eps) MAT(a,n,i,j) = 0;
    }
}

bool hqr(const Mat& a, int n, Mat& mz)
{
    /* find eigenvalues of upper hessenberg square Matrix `a' of
     * size nxn.
     * `mz' is a complex array
     */

    VALUE  z, y, x, w, v, u, t, s, r, q, p, norm;
    int nn,m,l,k,j,it,i,mn;
    VALUE eps = BCD::epsilon(19);

    norm = 0;
    for (j = 0; j < n; ++j) 
        norm += fabs(MAT(a,n,0,j));
    for (i = 1; i < n; ++i) 
    {
        for (j = i-1; j < n; ++j) 
            norm += fabs(MAT(a,n,i,j));
    }

    nn = n-1;
    t = 0;
    while (nn >= 0) 
    {
        it = 0;
        do 
        {
            squelchMat(a,n, eps*norm);
            //printMat(a,n);

            for (l=nn; l > 0; --l) 
            {
                if (MAT(a,n,l,l-1) == 0)
                        break;

                s = fabs(MAT(a,n,l-1,l-1))+fabs(MAT(a,n,l,l));
                if (s == 0)
                    s = norm;

                //if (MAT(a,n,l,l-1) + s == s) 
                if (fabs(MAT(a,n,l,l-1)) <= eps*s)
                {
#if 0
                    // additional check
                    VALUE s00 = MAT(a,n,l-1,l-1);
                    VALUE s01 = MAT(a,n,l-1,l);
                    VALUE s10 = MAT(a,n,l,l-1);
                    VALUE s11 = MAT(a,n,l,l);

                    VALUE p00 = fabs(s00);
                    VALUE p01 = fabs(s01);
                    VALUE p10 = fabs(s10);
                    VALUE p11 = fabs(s11);

                    VALUE ab = p10;
                    VALUE ba = ab;
                    if (p01 > ab) ab = p01; // max
                    if (p01 < ba) ba = p01; // min

                    VALUE aa = p11;
                    VALUE bb = aa;
                    VALUE d = fabs(s00 - s11);
                    if (d > aa) aa = d;
                    if (d < bb) bb = d;
                    
                    s = aa + ab;
                    if (ba*(ab/s) < eps*(bb*(aa/s)))
#endif

                    {
                        MAT(a,n,l,l-1) = 0;
                        break;
                    }
                }
            }

            x = MAT(a,n,nn,nn);

            bool found = l == nn;
            if (found)
            {
                VECC(mz, nn) = Complex(x+t);
                --nn;
            }
            else 
            {
                y = MAT(a,n,nn-1,nn-1);
                w = MAT(a,n,nn,nn-1)*MAT(a,n,nn-1,nn);
                if (l == nn-1) 
                {
                    found = true;
                    p = (y-x)/2;
                    q = p*p + w;
                    
                    z = sqrt(fabs(q));
                    x += t; 
                    if (q >= 0) 
                    {
                        z = p + SIGN(z,p);
                        if (z != 0) VECC(mz, nn) = Complex(x-w/z);
                        else VECC(mz,nn) = Complex(x);
                        VECC(mz, nn-1) = Complex(x+z);
                    }
                    else 
                    {
                        VECC(mz, nn) = Complex(x+p, z);
                        VECC(mz, nn-1) = Complex(x+p, -z);
                    }
                    nn -= 2;
                }
            }

            if (!found)
            {
                if (it == 40) 
                {
                    // printf("hqr, too many itarations\n");
                    return false;
                }
                if (t == 10 || it == 20 || it == 30)
                {
                    // exceptional shift
                    t += x;
                    for (i = 0; i <= nn; ++i) MAT(a,n,i,i) -= x;
                    s = fabs(MAT(a,n,nn,nn-1)) +
                        fabs(MAT(a,n,nn-1,nn-2));
                    y = x = s*VALUE(75)/VALUE(100);
                    w = s*s*VALUE(-4375)/VALUE(10000);
                }
                ++it;

                for (m = nn-2; m >= l; --m) 
                {
                    z = MAT(a,n,m,m);
                    r = x - z;
                    s = y - z;
                    p = (r*s-w)/MAT(a,n,m+1,m) + MAT(a,n,m,m+1);
                    q = MAT(a,n,m+1,m+1) - z - r - s;
                    r = MAT(a,n,m+2,m+1);
                    s = fabs(p) + fabs(q) + fabs(r);
                    p /= s;
                    q /= s;
                    r /= s;
                    if (m == l) break;
                    u = fabs(MAT(a,n,m,m-1))*(fabs(q)+fabs(r));
                    v = fabs(p)*(fabs(MAT(a,n,m-1,m-1))+
                                 fabs(z)+fabs(MAT(a,n,m+1,m+1)));

                    //if (u+v == v)
                    if (u <= eps*v)
                    {
                        //MAT(a,n,m,m-1) = 0; 
                        break;
                    }
                }

                for (i = m; i < nn-1; ++i) 
                {
                    MAT(a,n,i+2,i) = 0;
                    if (i != m) MAT(a,n,i+2,i-1) = 0;
                }
                    
                for (k = m; k < nn; ++k) 
                {
                    if (k != m) 
                    {
                        p = MAT(a,n,k,k-1);
                        q = MAT(a,n,k+1,k-1);
                        r = 0;
                        if (k != nn-1) r = MAT(a,n,k+2,k-1);
                        x = fabs(p)+fabs(q)+fabs(r);
                        if (x != 0) 
                        {
                            p /= x;
                            q /= x;
                            r /= x;
                        }
                    }
                        
                    s = SIGN(sqrt(p*p+q*q+r*r), p);
                    if (s != 0) 
                    {
                        if (k == m) 
                        {
                            if (l != m) 
                                MAT(a,n,k,k-1) = -MAT(a,n,k,k-1);
                        }
                        else 
                            MAT(a,n,k,k-1) = -s*x;

                        p += s;
                        x = p/s;
                        y = q/s;
                        z = r/s;
                        q /= p;
                        r /= p;
                        for (j = k; j <= nn; ++j) 
                        {
                            p = MAT(a,n,k,j)+q*MAT(a,n,k+1,j);
                            if (k != nn-1) 
                            {
                                p += r*MAT(a,n,k+2,j);
                                MAT(a,n,k+2,j) -= p*z;
                            }
                            MAT(a,n,k+1,j) -= p*y;
                            MAT(a,n,k,j) -= p*x;
                        }

                        mn = nn<k+3 ? nn : k+3;
                        for (i = l; i <= mn; ++i) 
                        {
                            p = x*MAT(a,n,i,k)+y*MAT(a,n,i,k+1);
                            if (k != nn-1) 
                            {
                                p += z*MAT(a,n,i,k+2);
                                MAT(a,n,i,k+2) -= p*r;
                            }
                            MAT(a,n,i,k+1) -= p*q;
                            MAT(a,n,i,k) -= p;
                        }
                    }
                }
            }
        } while (l < nn-1);
    }
    return true;
}

bool prootEigen(const Matrix& a, Matrix& rmat)
{
    // expect a vector of coefficients
    if (a._nRows != 1) return false;

    int m = a._nCols-1;  // polynomial degree

    while (VEC(a._a,m) == 0 && m > 0) --m;

    if (!m) return false;
    else if (m == 1)
    {
        // trivial
        Matrix::_create(rmat, 1, m);
        VEC(rmat._a,0) = -VEC(a._a,0)/VEC(a._a,1);
        return true;
    }
    else
    {
        // root results
        Matrix::_create(rmat, 1, m, true);   // complex result
    
        Matrix hess;
        Matrix::_create(hess, m, m);

        int k, j;
        for (k = 0; k < m; ++k)
        {
            MAT(hess._a, 0, 0, k) = -VEC(a._a,m-k-1)/VEC(a._a,m);
            for (j = 1; j < m; ++j) MAT(hess._a, 0, j, k) = 0;
            if (k != m-1) MAT(hess._a,0,k+1,k) = 1;
        }
        balance(hess._a, m);
        return hqr(hess._a, m, rmat._a);
    }
}



