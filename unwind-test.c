#include "unwind-test.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void
test_fp(struct _Unwind_Context *context)
{
    void *ra_con = (void *)_Unwind_GetIP(context);
    printf("ra_con is: 0x%016x\n", ra_con);
    void *ra = NULL;
    asm ("movq %%rbp, %0\n"
         : "=r" (ra));
    //printf("ra is: 0x%016x\n", ra);
    ra -= 8;
    long long *ra_long = (long long *)ra;
    long long raa = (long long) *ra_long;
    printf("ra is: 0x%016x\n", raa);
    return;
}

#ifdef __cplusplus
}
#endif