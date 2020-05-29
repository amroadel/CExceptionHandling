#include "test-unwind.h"
#include "stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*test_Unwind_Exception_Cleanup_Fn)
    (test_Unwind_Reason_Code,
    struct test_Unwind_Exception *);

struct test_Unwind_Exception {
    test_Unwind_Exception_Class exception_class;
    test_Unwind_Exception_Cleanup_Fn exception_cleanup;
    test_Unwind_Word private_1;
    test_Unwind_Word private_2;
};

struct test_Unwind_Context{
    void *ra;
    void *base;
    void *lsda;
};

test_Unwind_Reason_Code
test_Unwind_RaiseException(struct test_Unwind_Exception *exception_object)
{
    struct test_Unwind_Context context;
    void *ra;

    void *bp = NULL;
    asm ("movq %%rbp, %0\n"
         : "=r" (bp));

    do {
        bp = (void *) *((long *)bp);
        ra = (void *) *((long *)bp - 1);
        context.ra = ra;
        //test_Unwind(&context); // This should print ra for testing, it was done in a separate code
    } while (ra !=  0);
}

#ifdef __cplusplus
}
#endif