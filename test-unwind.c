#include "test-unwind.h"
#include "test-unwind-eh.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Data types*/
typedef void (*test_Unwind_Exception_Cleanup_Fn)
    (test_Unwind_Reason_Code,
    struct test_Unwind_Exception *);

struct test_Unwind_Exception {
    test_Unwind_Exception_Class exception_class;
    test_Unwind_Exception_Cleanup_Fn exception_cleanup;
    test_Unwind_Word private_1;
    test_Unwind_Word private_2;
};

/* Routines */
test_Unwind_Reason_Code
test_Unwind_RaiseException(struct test_Unwind_Exception *exc)
{
    struct test_Unwind_Context *context;
}

test_Unwind_Reason_Code
test_Unwind_ForcedUnwind(struct test_Unwind_Exception *exc, test_Unwind_Stop_Fn, void *stop);

void
test_Unwind_Resume(struct test_Unwind_Exception *exc);

void
test_Unwind_DeleteException(struct test_Unwind_Exception *exc);

#ifdef __cplusplus
}
#endif