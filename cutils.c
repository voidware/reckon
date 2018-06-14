#include "cutils.h"

int u_strnicmp(const char* s1, const char* s2, int n)
{
    int v = 0;
    int c1, c2;
    while (n) 
    {
        --n;

        c1 = *s1;
        c2 = *s2;

        if (u_isupper(c1)) c1 = u_tolower(c1);
        if (u_isupper(c2)) c2 = u_tolower(c2);
        
        v = (c2 - c1);
        if (v || !*s1) break;
        ++s1;
        ++s2;
    }
    return v;
}
