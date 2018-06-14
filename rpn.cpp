/**
 * Copyright (c) 2009 voidware ltd.
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

extern "C"
{
#include "fxlib.h"
#include "common.h"

extern void do_rpn();
}

#include "types.h"
#include "calc.h"

#define RPN_STACK_SIZE          8

static TermRef stack[RPN_STACK_SIZE];
static bool lift;

void InitRPN()
{
    lift = false;
    for (int i = 0; i < RPN_STACK_SIZE; ++i)
        stack[i].purge();

    stack[0] = Rational::create(0L);
}

static void Push()
{
    int i;
    for (i = RPN_STACK_SIZE-1; i > 0; --i)
        stack[i] = stack[i-1];
}

static void Pop()
{
    int i;
    for (i = 0; i < RPN_STACK_SIZE-1; ++i)
        stack[i] = stack[i+1];
}

static void RedrawStack(int n)
{
    // draw the stack
    int i;
    static TermRef str = String::create();

    AC();
    for (i = RPN_STACK_SIZE; i > n; --i)
    {
        if (stack[i-1])
        {
            STRING(str)->purge();
            Calc::theCalc->asString(stack[i-1], str);
            print(STRING(str)->s_);
        }
        if (i > 1) newline();
    }

    pos.x = 1; pos.y = 8;
    locate(pos.x, pos.y);
}

static void RedrawDisplay()
{
    RedrawStack(0);
}

bool DoNumber(unsigned int& k, char* buf, unsigned int size)

{
    char* p = buf;
    bool started = false;
    bool point = false;
    int ee = 0;
    bool allowall = false;

    while (allowall 
           || k >= '0' && k <= '9' || k == '.' 
           || (k >= 'A' && k <= 'Z')
           || (k >= 'a' && k <= 'z')
           || k == '#' 
           || k == '\'' || k == '[' || k == '('
           || ISEXP(k)
           || k == CUSTOM_PI_CHAR
           || k == KEY_CTRL_DEL
           || k == KEY_CHAR_PMINUS)
    {
        if (!started)
        {
            if (k == KEY_CTRL_DEL)
                return false; // wasnt a number start

            if (k == '\'' || k == '[' || k == '(')
            {
                // must be first character
                allowall = true;
            }

            if (lift) 
            {
                Push();
                RedrawStack(1); // dont draw X term yet.
            }
            lift = false;
            started = true;
            clearLine(7);
            if (k == KEY_CHAR_PMINUS) k = '-';

            if (ISEXP(k))
            {
                // start with EXP 
                *p++ = '1'; // force a 1
            }
        }

        if (k == KEY_CHAR_PMINUS && !allowall)
        {
            clearLine(7);
            char* pp = buf;
            if (ee) pp += ee;
            if (*pp == '-')
            {
                memmove(pp, pp+1, strlen(pp) + 1);
                --p;
            }
            else
            {
                memmove(pp + 1, pp, strlen(pp) + 1);
                *pp = '-';
                ++p;
            }
            print(buf);
        }
        else if (k == KEY_CTRL_EXE)
            break;
        else if (k == KEY_CTRL_AC)
            return false; // escape from allowall
        else if (k == KEY_CTRL_DEL)
        {
            // delete key
            if (p > buf)
            {
                if (p[-1] == '.') point = false;
                
                *--p = '\b'; printc(p);

                // deal with wide chars
                if (p > buf && ESC_CHAR(p[-1])) --p;

                if (p - buf < ee)
                    ee = 0;

                *p = 0;
            }
        }
        else if ((k == '\'' || k == '[' || k == '(') && !allowall)
            k = 0; // ignore unless at the start
        else if (p - buf < size-2) // space for wide char & 0
        {
            bool ok = true;
            if (k == '.')
            {
                ok = !point && !ee;
                point = true;
            }

            else if (k == KEY_CHAR_PMINUS)
                k = '-';
            else if (ISEXP(k))
                ok = !ee;

            if (allowall)
            {
                // expand like ALG
                const KeyWord* kw = findExpandKey(k, 1); // ALG
                if (kw)
                {
                    const char* s = kw->keyword;
                    while (*s && p - buf < size-1)
                    {
                        *p = *s++;
                        printc(p);
                        ++p;
                    }
                    k = 0;
                }
            }

            if (k && (ok || allowall))
            {
                char* q = p;
                if (k > 0xff) *p++ = k>>8;
                *p++ = k;
                *p = 0;
                printc(q);

                if (ISEXP(k))
                    ee = p - buf;
            }
        }
        k = GetKeyNonblocking(1, 0); // block
        k = translateKey(k);
    }
    *p = 0;

    if (started)
    {
        // check for illegal fragments.
        unsigned int l = strlen(buf);
        if (l == 1 && buf[0] == '-')
        {
            buf[0] = '0'; // turn it into a zero.
        }
    }
    return started;
}


static TermRef binOp(char c)
{
    char ops[2];
    ops[0] = c;
    ops[1] = 0;
    Function* f = 0;
    if (stack[0] && stack[1])
    {
        const char* p = ops;
        Operator* op = Operator::parse(&p, FUNC_INFIX);
        if (op) 
        {
            /* we have a binop functional */
            f = Function::create(2);
            f->symbol_ = op->symbol_;
            f->setArg(0, *stack[1]);
            f->prec_ = op->prec_;
            f->setArg(1, *stack[0]);
            f->flags_ = op->flags_;
            
            Pop();
            Pop();
        }
    }
    return f;
}

static TermRef fnOp(const char* name, int nargs)
{
    Function* f = 0;
    int i;
    bool ok = true;
    for (i = 0; i < nargs; ++i)
        if (!stack[i]) { ok = false; break; }

    if (ok)
    {
        const char* a = name;
        Symbol* sym = Symbol::parse(&a, Calc::theCalc->tc_);
        if (sym)
        {
            f = Function::create(nargs);
            f->nargs_ = nargs;
            f->symbol_ = sym;

            for (i = 0; i < nargs; ++i)
                f->setArg(i, *stack[nargs-i-1]);
            
            // pop stack too.
            for (i = 0; i < nargs;++i) Pop();

        }
    }
    return f;
}

static void update(TermRef& t)
{
    if (t)
    {
        bool eval = true;

        if (ISFUNCTION(t))
        {
            if (FUNCTION(t)->flags_ & FUNC_NOEVAL)
                eval = false;
        }

        if (eval)
            t = Calc::theCalc->eval(t);        

        if (t)
        {
            if (ISFUNCTION(t))
                FUNCTION(t)->fixPrecisions();
            
            if (lift) Push();
            stack[0] = t;
            RedrawDisplay();
            lift = true;
        }
    }
}

void do_rpn()
{
    unsigned int k;
    InitRPN();
    RedrawDisplay();

    for (;;)
    {
        TermRef t;

        k = GetKeyNonblocking(1, 0); // block
        k = translateKey(k);
        if (DoNumber(k, Calc::theCalc->inBuf, Calc::theCalc->inBufSize))
        {
            const char* p = Calc::theCalc->inBuf;
            t = Calc::theCalc->parse(&p);
            update(t);
            
            if (k == KEY_CTRL_EXE)
                continue;
        }
        else 
        {
            if (k == KEY_CTRL_EXE) // dup
            {
                Push();
                lift = true;
                RedrawDisplay();
                continue;
            }
        }

        // escape
        if (k == KEY_CTRL_OPTN) return;

        if (k == KEY_CTRL_AC)
        {
            InitRPN();
            RedrawDisplay();
        }
        else if (k == KEY_CTRL_DEL)
        {
            // perform drop
            Pop();
            lift = true;
            RedrawDisplay();
        }
        else if (k == KEY_CTRL_INS)
        {
            // perform swap
            if (stack[0] && stack[1])
            {
                TermRef t = stack[1];
                stack[1] = stack[0];
                stack[0] = t;
                lift = true;
                RedrawDisplay();
            }
        }
        else if (k == KEY_CTRL_HELP || k == KEY_CTRL_F1)
        {
            displayInfo();
            RedrawDisplay();
        }
        else if (k == '=')
        {
            // assign
            if (stack[0] && stack[1])
            {
                if (ISSYMBOL(stack[0]))
                {
                    SYMBOL(stack[0])->assign(*stack[1]);
                    Pop();
                    RedrawDisplay();
                    lift = true;
                }
            }
        }
        else
        {
            // see if we have a unary function
            const KeyWord* kw = findExpandKey(k, 2); // RPN
            if (kw)
            {
                if (kw->nargs >= 0)
                {
                    // function of some sort
                    t = fnOp(kw->keyword, kw->nargs);
                }
                else if (kw->nargs == -1)
                {
                    // symbol?
                    const char* p = kw->keyword;
                    t = Calc::theCalc->parse(&p);
                }
            }
            else
            {
                // try binop
                t = binOp(k);
            }

            update(t);
        }
    }
}
