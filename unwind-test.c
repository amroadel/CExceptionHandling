#include "unwind-test.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void
test_fp(struct _Unwind_Context *context)
{
    void *base = (void *)_Unwind_GetRegionStart(context);
    printf("base is: 0x%016x\n", base);
    void *ra_con = (void *)_Unwind_GetIP(context);
    printf("ra_con is: 0x%016x\n", ra_con);
    void *ra = NULL;
    asm ("movq %%rbp, %0\n"
         : "=r" (ra));
    //printf("ra is: 0x%016x\n", ra);
    //ra -= 8;
    long long *ra_long = (long long *)ra;
    long long ra0 = (long long) *ra_long;
    printf("ra0 is: 0x%016x\n", ra0);
    long long ra1 = (long long) *(long long *)ra0;
    printf("ra1 is: 0x%016x\n", ra1);
    long long ra2 = (long long) *(long long *)ra1; 
    printf("ra2 is: 0x%016x\n", ra2);
    long long ra3 = (long long) *(long long *)ra2;
    printf("ra2 is: 0x%016x\n", ra3);
    long long raf = (long long) *(long long *)ra2;
    printf("raf is: 0x%016x\n", raf);
    
    raf -= 8;
    long long ret = (long long) *(long long *)raf;
    printf("ret is: 0x%016x\n", ret);

    return;
}

#ifdef __cplusplus
}
#endif