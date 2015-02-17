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

/* also date & time calculations.
 * All datetimes use ISO 8601
 */

#include "finance.h"
#include "dpdmath.h"

typedef DPD VAL;

struct Date
{
    bool valid() const { return mo != 0; }

    int                 yr;
    int                 mo;
    int                 da;
    int                 mjd;
};

static long mjd( int y, int m, int d )
{
    // epoch 2000/1/1 
    int a = ( 14 - m ) / 12;
    m += 12 * a + 1;
    y += 4800 - a;
    return d + ( 153*m ) / 5 + 365L*y + y / 4 - y / 100 + y / 400 - 2483712L;
}

static void caldati( int mjd, int* y, int* m, int* d )
{
    // this is the inverse of mjd
    long td = mjd + 2520114L;
    unsigned int ty;
    unsigned int c, a;
    unsigned int tm;

    ty = ( td << 2 ) / 146097L;
    td -= ( 146097L * ty + 3 ) >> 2;
    c = ( 99 * ( td + 1 ) ) / 36160L;
    td -= (( 1461L * c ) >> 2 ) - 31;
    tm = ( 5 * td ) / 153;
    a = tm / 11;
    *d = td - ( 367L * tm ) / 12;
    *m = tm - 12 * a + 2;
    *y = 100 * ( ty - 49 ) + c + a;
}

static void from8601(const VAL& d, Date& date)
{
    memset(&date, 0, sizeof(date));
    int v = itrunc(d);
    if (v)
    {
        date.yr = v/10000;  v -= date.yr * 10000;
        date.mo = v/100;  
        date.da = v -= date.mo * 100;

        if (date.mo >= 1 && date.mo <= 12 &&
            date.da >= 1 && date.da <= 31)
        {
            date.mjd = mjd(date.yr, date.mo, date.da);
        }
        else
            date.mo = 0; // mark invalid
    }
}

static VAL to8601(Date& date)
{
    caldati(date.mjd, &date.yr, &date.mo, &date.da);
    return date.yr * 10000 + date.mo * 100 + date.da;
}

void mjdFloat(TermRef& res, Float* a)
{
    // convert ISO 8601 date to MJD.
    // eg 20140101

    Date d;
    from8601(a->v_, d); // &d
    if (d.valid())
        res = Float::create(d.mjd);
}

void dateFloat(TermRef& res, Float* a)
{
    // convert MJD to ISO6801
    Date d;
    d.mjd = itrunc(a->v_);
    res = Float::create(to8601(d));
}

static VAL window(Date& sDate, Date& mDate, int m, int* n)
{
    /* calculate the `window',
     *
     *           number of days after the settlement to next coupon
     *          --------------------------------------------------
     *                number of days in coupon period
     *
     * Also calculate the number of coupons pai
     *
     */

    int cm, cy;
    int tm, ty;
    int cstart, cend;

    /* find the first coupon date before the settlement. */
    cm = mDate.mo;
    cy = sDate.yr;
    if (mDate.mo > sDate.mo
        || (mDate.mo == sDate.mo && mDate.da > sDate.da)) --cy;

    /* nominal number of coupons */
    *n = (mDate.yr-cy)*m;
   
    tm = cm + 12/m;
    ty = cy;
    if (tm > 12)
    {
        tm -= 12;
        ++ty;
    }
    
    if (ty < sDate.yr
        || (ty == sDate.yr && tm < sDate.mo)
        || (ty == sDate.yr && tm == sDate.mo && mDate.da <= sDate.da))
    {
        /* still not after settlement */
        cm = tm;
        cy = ty;

        /* skip one coupon */
        --(*n);

        /* will after now */
        tm += 12/m;
        if (tm > 12)
        {
            tm -= 12;
            ++ty;
        }
    }
        
    /* coupon period start sate (<= settlement) */
    cstart = mjd(cy, cm, mDate.da);

    /* coupon period end date (> settlement, <= maturity) */
    cend = mjd(ty, tm, mDate.da);
    
    /* proportion of period inside settlement */
    return ((VAL)(cend - sDate.mjd))/(cend - cstart);
}

static VAL _price(const VAL& call, const VAL& cpn, const VAL& y,
                  const VAL& n, const VAL& w)
{
    /* calculate the bond price */
    if (!y.isZero())
    {
        VAL t = pow(1+y,1-n-w);
        return cpn*(w + t*(pow(1+y,n)-1)/y - 1) + call*t;
    }
    else return call+cpn*(n+w-1);
}


void bondPriceFloat(TermRef& res,
                    Float* settlement,
                    Float* maturity,
                    Float* cpn,
                    Float* y,
                    Float* call,
                    Float* m)
{
    Date sDate, mDate;
    from8601(settlement->v_, sDate);
    from8601(maturity->v_, mDate);
    if (sDate.valid() && mDate.valid())
    {
        int n;
        int mi = itrunc(m->v_);
        VAL w = window(sDate, mDate, mi, &n);

        /* convert percentage to rate per coupon period */
        VAL yv = y->v_/100/m->v_;

        /* `cpn' is annual, convert to per coupon period */
        VAL cpnv = cpn->v_/m->v_;
        
        VAL pv = _price(call->v_, cpnv, yv, VAL(n), w);
        res = Float::create(pv);
    }
}

VAL _modifiedDuration(const VAL& call, const VAL& cpn, const VAL& y,
                      const VAL& n, const VAL& w)
{
    // modified duration
    VAL md;
    if (!y.isZero())
    {
        VAL t2 = pow(y+1,n);
        md = cpn*((t2 - 1)*(w*y + 1) - n*y);
        md += call*y*y*(n + w - 1);
        md /= y*(y+1)*((t2-1)*cpn+call*y);
    }
    else
    {
        VAL d = call+cpn*n;
        if (!d.isZero())
        {
            md = 2*call*(n+w-1);
            md += cpn*n*(2*w+n-1);
            md /= 2*d;
        }
        else
            md = d;
    }
    return md;
}

void bondMDurationFloat(TermRef& res,
                       Float* settlement,
                       Float* maturity,
                       Float* cpn,
                       Float* y,
                       Float* call,
                       Float* m)
{
    Date sDate, mDate;
    from8601(settlement->v_, sDate);
    from8601(maturity->v_, mDate);
    if (sDate.valid() && mDate.valid())
    {
        int n;
        int mi = itrunc(m->v_);
        VAL w = window(sDate, mDate, mi, &n);

        /* convert percentage to rate per coupon period */
        VAL yv = y->v_/100/m->v_;

        /* `cpn' is annual, convert to per coupon period */
        VAL cpnv = cpn->v_/m->v_;
        
        VAL md = _modifiedDuration(call->v_,
                                   cpnv,
                                   yv,
                                   VAL(n),
                                   w) / m->v_;
        res = Float::create(md);
    }
}



