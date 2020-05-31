#include "test-unwind.h"
#include "test-unwind-eh.h"
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

struct test_Unwind_Context {
    void *cfa;
    void *ra;
    void *lsda;
    struct dwarf_bases bases;
    // keep them for now until we know more about them
    test_Unwind_Word flags;
    test_Unwind_Word version;
    test_Unwind_Word args_size;
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