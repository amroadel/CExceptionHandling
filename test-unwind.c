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
test_Unwind_RaiseException(struct test_Unwind_Exception *exc)
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

test_Unwind_Reason_Code
test_Unwind_ForcedUnwind(struct test_Unwind_Exception *exc, test_Unwind_Stop_Fn, void *stop);

void
test_Unwind_Resume(struct test_Unwind_Exception *exc);

void
test_Unwind_DeleteException(struct test_Unwind_Exception *exc);

test_Unwind_Word
test_Unwind_GetGR(struct test_Unwind_Context *context, int index);

void
test_Unwind_SetGR(struct test_Unwind_Context *context, int index, test_Unwind_Word val);

test_Unwind_Ptr
test_Unwind_GetIP(struct test_Unwind_Context *context)
{
    return (test_Unwind_Ptr) context->ra;
}

void
test_Unwind_SetIP(struct test_Unwind_Context *context, test_Unwind_Ptr val)
{
    context->ra = (void *)val;
}

test_Unwind_Word
test_Unwind_GetCFA(struct test_Unwind_Context *context)
{
    return (test_Unwind_Word) context->cfa;
}

test_Unwind_Ptr
test_Unwind_GetLanguageSpecificData(struct test_Unwind_Context *context)
{
    return (test_Unwind_Ptr) context->lsda;
}

test_Unwind_Ptr
test_Unwind_GetRegionStart(struct test_Unwind_Context *context)
{
    return (test_Unwind_Ptr) context->bases.func;
}

test_Unwind_Ptr
test_Unwind_GetTextRelBase(struct test_Unwind_Context *context)
{
    return (test_Unwind_Ptr) context->bases.tbase;
}

test_Unwind_Ptr
test_Unwind_GetDataRelBase(struct test_Unwind_Context *context)
{
    return (test_Unwind_Ptr) context->bases.dbase;
}

#ifdef __cplusplus
}
#endif