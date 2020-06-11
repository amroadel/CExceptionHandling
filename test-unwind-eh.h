#ifndef TEST_UNWIND_EH
#define TEST_UNWIND_EH

#include "test-unwind.h"
#include "test-unwind-fde.h" // remember to remove this if we got rid of test_extract_cie_info dependancy, which is preferable

#ifdef __cplusplus
extern "C" {
#endif

/* Data types*/
struct test_Unwind_FrameState;
typedef struct test_Unwind_FrameState test_Unwind_FrameState;

/* Routines */
void
add_lsda(const unsigned char *fde, struct test_Unwind_Context *context);

test_Unwind_Reason_Code
test_uw_frame_state_for (struct test_Unwind_Context *context, test_Unwind_FrameState *fs);

const unsigned char *
test_extract_cie_info (const struct test_dwarf_cie *cie, struct test_Unwind_Context *context,
  test_Unwind_FrameState *fs);


#ifdef __cplusplus
}
#endif

#endif /* TEST_UNWIND_EH */