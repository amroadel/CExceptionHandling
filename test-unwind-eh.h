#ifndef TEST_UNWIND_EH
#define TEST_UNWIND_EH

#include "test-unwind.h"
#include "test-unwind-fde.h" // TODO: remember to remove this if we got rid of test_extract_cie_info dependancy, which is preferable

#ifdef __cplusplus
extern "C" {
#endif

typedef void * test_Unwind_Context_Reg_Val;
typedef unsigned test_Unwind_Internal_Ptr __attribute__((__mode__(__pointer__)));

/* Data types*/
struct test_Unwind_FrameState;
typedef struct test_Unwind_FrameState test_Unwind_FrameState;

/* Routines */
void __attribute__((noinline))
init_context(struct test_Unwind_Context *context, void *outer_cfa, void *outer_ra);

void
add_lsda(const unsigned char *fde, struct test_Unwind_Context *context);

test_Unwind_Reason_Code
test_uw_frame_state_for(struct test_Unwind_Context *context, test_Unwind_FrameState *fs);

const unsigned char *
test_extract_cie_info(const struct test_dwarf_cie *cie, struct test_Unwind_Context *context,
    test_Unwind_FrameState *fs);

#ifdef __cplusplus
}
#endif

#endif /* TEST_UNWIND_EH */