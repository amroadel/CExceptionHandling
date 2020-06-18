#include "test-unwind.h"
#include "test-unwind-eh.h"
#include "stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

/*  Data types  */
typedef void (*test_Unwind_Exception_Cleanup_Fn)
    (test_Unwind_Reason_Code,
    struct test_Unwind_Exception *);

struct test_Unwind_Exception {
    test_Unwind_Exception_Class exception_class;
    test_Unwind_Exception_Cleanup_Fn exception_cleanup;
    test_Unwind_Word private_1;
    test_Unwind_Word private_2;
};

/*  Routines  */
test_Unwind_Reason_Code
test_Unwind_RaiseException_Phase2(struct test_Unwind_Exception *exc,
    struct test_Unwind_Context *context,
    unsigned long *frames_p);

test_Unwind_Reason_Code
test_Unwind_RaiseException(struct test_Unwind_Exception *exc)
{
    struct test_Unwind_Context *this_context, *cur_context;
    test_Unwind_Reason_Code code;
    unsigned long frames;

    /*  Phase 1: Unwind the stack, calling the personality routine
        with the _UA_SEARCH_PHASE flag set.
        Do not modify the stack yet.  */
    
    /*  Step 1: initialize the context to describe the current frame.  */
    test_uw_init_context(this_context);
    uw_copy_context(cur_context, this_context);

    while (1) {
        /*  Step 2: set up the frame state to describe the caller of cur_context.  */
        test_Unwind_FrameState *fs;
        code = test_uw_frame_state_for(cur_context, fs);
        /*  Hit end of stack with no handler found.  */
        if (code == _URC_END_OF_STACK)
            return _URC_END_OF_STACK;
        /*  Some error encountered.  */
        if (code != _URC_NO_REASON)
            return _URC_FATAL_PHASE1_ERROR;

        /*  Step 3: run the personality routine.  */
        if (uw_get_personality(fs)) {
            code = uw_get_personality(fs)(1, _UA_SEARCH_PHASE,
                exc->exception_class, exc, cur_context);
            if (code == _URC_HANDLER_FOUND)
                break;
            else if (code != _URC_CONTINUE_UNWIND)
                return _URC_FATAL_PHASE1_ERROR;
        }

        /*  Step 4: update the cur_context using frame state fs  */
        test_uw_update_context (cur_context, fs);
        free(fs);
    }

    /*  Indicate to _Unwind_Resume and associated subroutines that this
        is not a forced unwind using private 1.
        Note where we found a handler using private 2.  */
    exc->private_1 = 0;
    exc->private_2 = test_uw_identify_context(cur_context);

    /*  Phase 2: Unwind the stack agian, calling the personality routine
        with the _UA_CLEANUP_PHASE flag set this time.
        Find cleanup code and execute it.
        We'll only locate the first such frame here.
        Cleanup code will call back into test_Unwind_Resume and we'll continue Phase 2 there.  */
    uw_copy_context(cur_context, this_context);
    code = test_Unwind_RaiseException_Phase2(exc, cur_context, &frames);
    if (code != _URC_INSTALL_CONTEXT)
        return code;
    test_uw_install_context(this_context, cur_context, frames);
    //TODO: if install context can free the context objects, do it there
    free(cur_context);
    free(this_context);
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