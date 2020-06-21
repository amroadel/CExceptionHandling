#ifndef TEST_UNWIND_EH
#define TEST_UNWIND_EH

#include "test-unwind.h"

#ifdef __cplusplus
extern "C" {
#endif

/*  Special Handling: Intel CET technology support and shadow stack handling.  */
#define test_Unwind_Frames_Extra(x)

typedef void * test_Unwind_Context_Reg_Val;
typedef unsigned test_Unwind_Internal_Ptr __attribute__((__mode__(__pointer__)));

/*  Data types  */
struct test_Unwind_FrameState_t;
typedef struct test_Unwind_FrameState_t test_Unwind_FrameState;

/*  Routines  */
void
test_Unwind_DebugHook(void *cfa __attribute__ ((__unused__)),
    void *handler __attribute__ ((__unused__)));

void __attribute__((noinline))
_init_context(struct test_Unwind_Context *context, void *outer_cfa, void *outer_ra);

void
test_uw_update_context(struct test_Unwind_Context *context, test_Unwind_FrameState *fs);

test_Unwind_Ptr
test_uw_identify_context(struct test_Unwind_Context *context);

void
uw_copy_context(struct test_Unwind_Context *target, struct test_Unwind_Context *source);

long
_install_context(struct test_Unwind_Context *current, struct test_Unwind_Context *target);

void *
test_uw_frob_return_addr(struct test_Unwind_Context *current __attribute__ ((__unused__)),
    struct test_Unwind_Context *target);

void
uw_free_context(struct test_Unwind_Context * context);

test_Unwind_Reason_Code
test_uw_frame_state_for(struct test_Unwind_Context *context, test_Unwind_FrameState *fs);

test_Unwind_Personality_Fn
uw_get_personality(test_Unwind_FrameState *fs);

/*  Fill in CONTEXT for top-of-stack.  The only valid registers at this
    level will be the return address and the CFA.  */
#define test_uw_init_context(CONTEXT)                                           \
do {                                                                            \
    __builtin_unwind_init();                                                    \
    _init_context(CONTEXT, __builtin_dwarf_cfa(), __builtin_return_address(0)); \
} while (0) //TODO: unwind_init appears to be practicallly useless. Try it once the library is done

/*  Install TARGET into CURRENT so that we can return to it.  This is a
    macro because __builtin_eh_return must be invoked in the context of
    our caller.  FRAMES is a number of frames to be unwind.
    _Unwind_Frames_Extra is a macro to do additional work during unwinding
    if needed, for example shadow stack pointer adjustment for Intel CET
    technology.  */
#define test_uw_install_context(CURRENT, TARGET, FRAMES)                        \
do {                                                                            \
    long offset = _install_context((CURRENT), (TARGET));                        \
    void *handler = test_uw_frob_return_addr((CURRENT), (TARGET));              \
    test_Unwind_DebugHook(test_Unwind_GetCFA(TARGET), handler);                 \
    uw_free_context(CURRENT);                                                   \
    uw_free_context(TARGET);                                                    \
    test_Unwind_Frames_Extra(FRAMES);					                        \
    __builtin_eh_return(offset, handler);                                       \
} while (0)

#ifdef __cplusplus
}
#endif

#endif /*  TEST_UNWIND_EH  */