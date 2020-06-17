#ifndef TEST_UNWIND_EH
#define TEST_UNWIND_EH

#include "test-unwind.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void * test_Unwind_Context_Reg_Val;
typedef unsigned test_Unwind_Internal_Ptr __attribute__((__mode__(__pointer__)));

/* Data types*/
struct test_Unwind_FrameState_t;
typedef struct test_Unwind_FrameState_t test_Unwind_FrameState;

/* Routines */
#define test_uw_init_context(CONTEXT)                                           \
do {                                                                            \
    __builtin_unwind_init();                                                    \
    init_context(CONTEXT, __builtin_dwarf_cfa(), __builtin_return_address(0));  \
} while (0) //TODO: unwind_init appears to be practicallly useless. Try it once the library is done

void __attribute__((noinline))
init_context(struct test_Unwind_Context *context, void *outer_cfa, void *outer_ra);

void
add_lsda(const unsigned char *fde, struct test_Unwind_Context *context);

test_Unwind_Reason_Code
test_uw_frame_state_for(struct test_Unwind_Context *context, test_Unwind_FrameState *fs);

#ifdef __cplusplus
}
#endif

#endif /* TEST_UNWIND_EH */