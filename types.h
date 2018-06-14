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

#ifndef __types_h__
#define __types_h__

#include <string.h>
#include "defs.h"
#include "dpd.h"
#include "stream.h"
#include "customchars.h"

#include "mi.h"
#include "complex.h"

#define EXPRESSION_TYPE         1
#define NUMBER_TYPE             2
#define REAL_TYPE               (3*NUMBER_TYPE)
#define COMPLEX_TYPE            (5*NUMBER_TYPE)
#define SYMBOL_TYPE             7
#define LSYMBOL_TYPE            (11*SYMBOL_TYPE)
#define STRING_TYPE             13
#define RATIONAL_TYPE           (17*NUMBER_TYPE)
#define FUNCTION_TYPE           29
#define LABEL_TYPE              (31*STRING_TYPE)
#define OPERATOR_TYPE           37
#define FLOAT_TYPE              (41*REAL_TYPE)
#define ARRAY_TYPE              43

/* 53, 59, 61 */

#define ISFUNCTION(_x)          ((_x)->type() == FUNCTION_TYPE)
#define ISOPERATOR(_x)          ((_x)->type() == OPERATOR_TYPE)
#define ISSYMBOL(_x)            ((_x)->type() == SYMBOL_TYPE)
#define ISSTRING(_x)            ((_x)->type() == STRING_TYPE)
#define ISLABEL(_x)             ((_x)->type() == LABEL_TYPE)
#define ISRATIONAL(_x)          ((_x)->type() == RATIONAL_TYPE)
#define ISFLOAT(_x)             ((_x)->type() == FLOAT_TYPE)
#define ISARRAY(_x)             ((_x)->type() == ARRAY_TYPE)
#define ISCOMPLEX(_x)           ((_x)->type() == COMPLEX_TYPE)

#define R(_x) ((Rational*)(_x))
#define F(_x) ((Float*)(_x))

#define MAX_FUNCTIONS           256
#define MAX_OPERATORS           16
#define MAX_FNARGS              8

/* function flags */
#define FUNC_NOFIX              0
#define FUNC_INFIX              1
#define FUNC_PREFIX             2
#define FUNC_POSTFIX            4
#define FUNC_SOMEFIX            7 
#define FUNC_NOEVAL             8

/* operator precedences */
#define PREC_BRACKETS           9
#define PREC_FUNCTION           8
#define PREC_UNARYMINUS         7
#define PREC_OPPOSTFIX          6
#define PREC_OPPOWER            5
#define PREC_OPPPARALLEL        4
#define PREC_OPMULDIV           3
#define PREC_OPADDSUB           2
#define PREC_QUOTE              1
#define PREC_ASSIGN             0

/* Forward Decls */
namespace Utils { struct DList; };

struct String;
struct Term;
struct RegInfo;
struct SymbolTable;
struct Symbol;
struct Operator;

/* function decls */
extern void InitSymbols();
extern void InitFunctions();

#define SYMBOL(_x)             ((Symbol*)((_x).ref_))
#define OPERATOR(_x)           ((Operator*)((_x).ref_))
#define FLOAT(_x)              ((Float*)((_x).ref_))
#define STRING(_x)             ((String*)((_x).ref_))
#define LABEL(_x)              ((Label*)((_x).ref_))
#define FLOAT(_x)              ((Float*)((_x).ref_))
#define ARRAY(_x)              ((Array*)((_x).ref_))
#define FUNCTION(_x)           ((Function*)((_x).ref_))
#define COMPLEX(_x)            ((ComplexN*)((_x).ref_))

typedef unsigned int Type;

class DispFormat
{
public:

    DispFormat()
    {
        _format = BCDFloat::format_rounding;
        _precision = 0;
    }

    unsigned int precision() const { return _precision; }
    void precision(unsigned int v) { _precision = v; }

    BCD::Format         _format;
    unsigned int        _precision;
};


struct TermRef
{
    // Constructors
                                TermRef() { ref_ = 0; }
                                TermRef(const Term*);
                                TermRef(const TermRef& r)
                                        { ref_ = 0; *this = r; }

    // Destructor
                                ~TermRef() { purge(); }

    // Features
    TermRef&                    operator=(const TermRef&);
                                operator bool() const { return ref_ != 0; }
    Term*                       operator->() const { return ref_; }
    Term*                       operator*() const { return ref_; }
    void                        purge();
    bool                        valid() const { return ref_ != 0; }

    bool                        operator==(const TermRef& t) const
                                        { return ref_ == t.ref_; }
    bool                        operator!=(const TermRef& t) const
                                        { return ref_ != t.ref_; }
    
    Term*                       ref_;
};

typedef void FnImpl(TermRef& res, ...);
typedef void FnImpl0(TermRef& res);
typedef void FnImpl1(TermRef& res, Term*);
typedef void FnImpl2(TermRef& res, Term*, Term*);
typedef void FnImpl3(TermRef& res, Term*, Term*, Term*);
typedef void FnImpl4(TermRef& res, Term*, Term*, Term*, Term*);
typedef void FnImpl5(TermRef& res, Term*, Term*, Term*, Term*, Term*);
typedef void FnImpl6(TermRef& res, Term*, Term*, Term*, Term*, Term*, Term*);
struct TermContext
{
    // Constructor
                                TermContext();

    // Features
    Symbol*                     internSymbol(const char*, unsigned int l);
    void                        push();
    SymbolTable*                pop();

private:

    SymbolTable*                sTab_;
};

struct Term
{
    // Constructors
                                Term() { refCount_ = 0; }
    virtual                     ~Term() {}

    virtual Type                type() const = 0;
    virtual void                asString(TermRef&, DispFormat*) const = 0;
    virtual bool                reduce(TermRef& res, TermContext&)
    {
        res = this; 
        return false;
    }
    static Type                 mytype() { return 1; }
    virtual TermRef             clone() = 0;

    // Features

    bool                        convert(TermRef& res, Type b) const;

    static RegInfo*             findAnyFunction(Symbol* symbol);
    static Term*                parse(const char**, TermContext&);
    static void                 destroy(Term*);
    void                        incRef() { ++refCount_; }
    unsigned int                decRef() { return --refCount_; }

    int                         refCount_;
};

inline TermRef::TermRef(const Term* o)
    : ref_(const_cast<Term*>(o))
{
    if (ref_) ref_->incRef();
}

struct String: public Term
{
    // Destructor
                                ~String() { purge(); }

    // Term compliance
    Type                        type() const { return mytype(); }
    static Type                 mytype() { return STRING_TYPE; }
    void                        asString(TermRef& s, DispFormat*) const;
    TermRef                     clone();

    // Features
    static String*              parse(const char**);
    static String*              create();
    void                        enoughFor(unsigned int);
    void                        enoughMore(unsigned int l)
                                        { enoughFor(size_ + l); }
    void                        set(const char* s, unsigned int l);
    void                        append(const char*);
    void                        append(char);
    void                        truncate(int n);
    void                        purge() { delete s_; init(); }
    void                        init();

    char*                       s_;
    int                         size_;
    int                         space_;
};

struct Label: public String
{
    /* a label is like a string, but used internally for annotation
     */

    // Term compliance
    Type                        type() const { return mytype(); }
    static Type                 mytype() { return LABEL_TYPE; }
    void                        asString(TermRef& s, DispFormat*) const;
    TermRef                     clone();

    // Features
    static Label*               create();

    int                         drawBrackets_;
    
};

struct Symbol: public Term
{
    // Constructors
                                Symbol() { init(); }
    // Destructor
                                ~Symbol();
    // Term compliance
    Type                        type() const { return mytype(); }
    static Type                 mytype() { return SYMBOL_TYPE; }
    void                        asString(TermRef& s, DispFormat*) const
                                        { STRING(s)->append(name_); }
    bool                        reduce(TermRef& res, TermContext&);
    TermRef                     clone() { return this; }


    // Features
    bool                        operator==(const Symbol& s) const
                                        { return !strcmp(name_, s.name_); }
    Operator*                   isOperator() const;

    static bool                 scan(const char** s);
    static Symbol*              parse(const char**, TermContext&);
    static Symbol*              create(const char*, unsigned int);
    void                        assign(Term*);
    bool                        assignOnly(Term*);
    bool                        isBound() const { return v_.valid(); }
    void                        unbind() { v_.purge(); }

    void                        init() { name_ = 0; table_ = 0; }

    char*                       name_;
    TermRef                     v_;
    SymbolTable*                table_;
};

#define ARG(_b, _a)  (((Term**)(_b + 1))[_a])
#define SET_ARG(_a, _v)  { (_a) = (_v); (_v)->incRef(); }
#define DROP_ARG(_a)                                    \
{                                                       \
    if ((_a) && !(_a)->decRef()) Term::destroy(_a);     \
    (_a) = 0;                                           \
}

struct Function: public Term
{
    enum flagsT 
    {
        flVoid,
    };

    // Constructors
                                Function(unsigned int nargs)
                                    : nargs_(nargs) { init(); }
    // Destructor
                                ~Function();

    // Term compliance
    Type                        type() const { return mytype(); }
    static Type                 mytype() { return FUNCTION_TYPE; }
    bool                        parse(const char**);
    void                        asString(TermRef&, DispFormat*) const;
    bool                        reduce(TermRef& res, TermContext&);
    TermRef                     clone();

    // Features
    void*                       operator new(size_t amt, int nargs)
                                { return ::new char[amt + 
                                                    nargs*sizeof(Term*)];
                                }
    void                        operator delete(void* p, int)
                                    { ::delete (char*)p; }

    void                        operator delete(void* p)
                                    { ::delete (char*)p; }

    static Function*            parse(const char**, TermContext&);
    static bool                 scan(const char**,
                                     const char** args, int* nargs);
    static Function*            create(int nargs);

    unsigned int                size() const { return nargs_; }

    bool                        isOperator() const
                                { return (flags_ & FUNC_SOMEFIX) != 0; }

    void                        init();
    void                        dropArg(unsigned int i) 
    {
        /* args are not references. */
        DROP_ARG(ARG(this, i));
    }

    void                        setArg(unsigned int i, Term* o)
    { SET_ARG(ARG(this, i), o); }
    
    void                        changeArg(unsigned int i, Term* o)
                                        { dropArg(i);  setArg(i, o); }
    void                        bind(Utils::DList&, Term** args);
    void                        fixPrecisions();


    void                        _fixPrecisions();
    bool                        _reduce(TermRef& res,
                                        RegInfo* binding, Term** argBuf);

    TermRef                     symbol_;
    unsigned short              nargs_;
    unsigned short              flags_;
    short                       prec_; /* when an operator */
    unsigned short              cosmeticBrackets_;
    RegInfo*                    binding_;
};

struct Number: public Term
{
    // Features
    static Number*              parse(const char**);

    TermRef                     clone() { return this; }
};

struct Real: public Number
{
    // Features
    static Real*                parse(const char**);
    static bool                 scan(const char**);
};

struct ComplexN: public Number
{
    // Term compliance
    Type                         type() const { return mytype(); }
    static Type                  mytype() { return COMPLEX_TYPE; }

    // Compliance, Term
    void                         asString(TermRef&, DispFormat*) const;

    // Features
    static ComplexN*             parse(const char**);
    static ComplexN*             create() { return new ComplexN; }
    static ComplexN*             create(const Complex& cmf)
    {
        ComplexN* c = new ComplexN;
        c->cmf_ = cmf;
        return c;
    }

    Complex                      cmf_;
};

struct Float: public Real
{
    // Term compliance
    Type                        type() const { return mytype(); }
    static Type                 mytype() { return FLOAT_TYPE; }
    void                        asString(TermRef&, DispFormat*) const;

    // Features
    static Float*              parse(const char**);
    static Float*              create() { return new Float; }
    TermRef                    clone() { return create(v_); }
    static Float*              create(const DPD& v)
                                {
                                    Float* m = new Float;
                                    m->v_ = v;
                                    return m;

                                }
    DPD                         v_;
};

struct Rational: public Number
{
    // Destructor
                                ~Rational();

    // Term Compliance
    Type                        type() const { return mytype(); }
    static Type                 mytype() { return RATIONAL_TYPE; }

    static Rational*            parse(const char**);
    static bool                 scan(const char** s);
    static Rational*            create()
                                {
                                    Rational* r = new Rational;
                                    r->rat_.x_ = 0;
                                    r->rat_.y_ = 0;
                                    return r;
                                }
    static Rational*            create(long v)
                                {
                                    Rational* r = new Rational;
                                    r->rat_ = BigFrac(v);
                                    return r;
                                }
    static Rational*            create(Big* a,  Big* b)
                                {
                                    Rational* r = new Rational;
                                    r->rat_.x_ = a;
                                    r->rat_.y_ = b;
                                    return r;
                                }
    static Rational*            create(Big* a)
                                {
                                    Rational* r = new Rational;
                                    r->rat_.x_ = a;
                                    r->rat_.y_ = one();
                                    return r;
                                }
    void                        asString(TermRef&, DispFormat*) const;

    BigFrac                     rat_;
};

struct Operator: public Term
{
    // Term compliance
    Operator()
    {
        prec_ = 0;
        similar_ = 0;
    }

    Type                        type() const { return mytype(); }
    static Type                 mytype() { return OPERATOR_TYPE; }
    void                        asString(TermRef& s, DispFormat*) const 
                                        { STRING(s)->append("operator"); }
    TermRef                     clone() { return this; }

    // Features
    static Operator*            create() { return new Operator(); }
    static Operator*            parse(const char**, int);

    unsigned int                nargs_;
    unsigned int                flags_;
    int                         prec_;
    TermRef                     symbol_;
    Operator*                   similar_;  // another with same name
};


struct Array: public Term
{
    Array() 
    {
        flags_ = 0;
    }

    ~Array() { _purge(); }

    enum Flags
    {
        array_normal = 0,
        array_realvector = 1,
        array_realmatrix = 2,
        array_complexvector = 5,
        array_complexmatrix = 6,
    };

    // Term compliance
    Type                        type() const { return mytype(); }
    static Type                 mytype() { return ARRAY_TYPE; }
    void                        asString(TermRef& s, DispFormat*) const;
    TermRef                     clone();
    bool                        reduce(TermRef& res, TermContext&);

    void*                       operator new(size_t amt, int n);
    void                        operator delete(void* p, int n)
                                        { ::delete (char*)p; }
    void                        operator delete(void* p)
                                        { ::delete (char*)p; }

    // Features
    static Array*               create(int);
    static Array*               parse(const char**, TermContext&);
    unsigned int                size() const { return size_; }
    bool                        isVecMat() const { return flags_ != 0; }
    bool                        isRealVecMat() const
                                { return flags_ != 0 && (flags_ & 0x4) == 0; }

    /* element access with no bounds checking.
     * use ONLY when limits already checked AND subtype is also
     * known to be array (for row,col).
     */
    TermRef&                    _at(int i) { return elts_[i]; }
    TermRef&                    _at(int row, int col)
    {
        return ARRAY(elts_[row])->elts_[col];
    }

    void                        _initFlags();
    void                        _purge()
    {
        for (int i = 0; i < size_; ++i)
        { elts_[i].purge(); }
    }

    unsigned short              size_; // number of elements
    unsigned short              flags_;
    unsigned short              nCols_; // if matrix
    TermRef                     elts_[1]; // first element
};

/****************************************************************/

struct RegInfo
{
    enum {
        convFnFlag = 1,
    };

    // Constructors
                                RegInfo(unsigned int nargs)
                                    : nargs_(nargs) { init(); }

    // Accessors
    bool                        convFn() const
                                  { return (flags_ & convFnFlag) != 0; }
    // Modifiers
    void                        setConvFn() { flags_ |= convFnFlag; }

    // Features
    static RegInfo*             create(Symbol* s,
                                       Type rType,
                                       int nargs, ...);
    static RegInfo*             createN(Symbol* s,
                                        Type rt,
                                        int nargs, const Type*);
    void*                       operator new(size_t amt, int nargs)
                                { return ::new char[amt +
                                                    (nargs-1)*sizeof(Type)];
                                }
    void                        operator delete(void* p, int)
                                        { ::delete (char*)p; }
    void                        operator delete(void* p)
                                        { ::delete (char*)p; }
    void                        init() { impl_ = 0; flags_ = 0; }


    TermRef                     symbol_;
    unsigned short              nargs_;
    unsigned short              flags_;
    FnImpl*                     impl_;
    Type                        rType_;
    Type                        argType_[1];
};

struct FnRegistry
{
    // Constructors
                                FnRegistry() { size_ = 0; }

    // Destructor
                                ~FnRegistry() { purge(); }

    bool                        intern(RegInfo* ri);
    void                        purge();
    RegInfo*                    findConverter(Type a, Type b) const;

    RegInfo*                    fn_[MAX_FUNCTIONS];
    unsigned int                size_;
};

struct Fn0ImplRec
{
    const char*         name_;
    Type                rt_;
    FnImpl*             impl_;
};

struct Fn1ImplRec
{
    const char*         name_;
    Type                rt_;
    FnImpl1*            impl_;
    Type                t1_;
};

struct Fn2ImplRec
{
    const char*         name_;
    Type                rt_;
    FnImpl2*            impl_;
    Type                t1_;
    Type                t2_;
};

struct FnnImplRec
{
    const char*         name_;
    Type                rt_;
    FnImpl*             impl_;
    int                 nargs_;
    Type                t_[8];  // max 8
};

struct TermList
{
    // Constructors
                        TermList() { init(); }
    
    // Destructor
                        ~TermList() { empty(); }

    // Features
    void                sizeFor(unsigned int);
    void                add(const TermRef&);
    void                insert(unsigned int at, const TermRef&);
    void                empty();
    void                init();

    unsigned int        size_;
    unsigned int        space_;
    TermRef*            terms_;
};

extern TermContext* currentTC;

#endif // types_h__
