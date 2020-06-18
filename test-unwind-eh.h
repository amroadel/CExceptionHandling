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
    _init_context(CONTEXT, __builtin_dwarf_cfa(), __builtin_return_address(0)); \
} while (0) //TODO: unwind_init appears to be practicallly useless. Try it once the library is done

void __attribute__((noinline))
_init_context(struct test_Unwind_Context *context, void *outer_cfa, void *outer_ra);

void
test_uw_update_context(struct test_Unwind_Context *context, test_Unwind_FrameState *fs);

inline test_Unwind_Ptr
test_uw_identify_context(struct test_Unwind_Context *context);

inline void
uw_copy_context(struct test_Unwind_Context *target, struct test_Unwind_Context *source);

test_Unwind_Reason_Code
test_uw_frame_state_for(struct test_Unwind_Context *context, test_Unwind_FrameState *fs);

inline test_Unwind_Personality_Fn
uw_get_personality(test_Unwind_FrameState *fs);

#ifdef __cplusplus
}
#endif

#endif /* TEST_UNWIND_EH */