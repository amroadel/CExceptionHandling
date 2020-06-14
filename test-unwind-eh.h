#ifndef TEST_UNWIND_EH
#define TEST_UNWIND_EH

#include "test-unwind.h"
#include "test-unwind-fde.h" // TODO: remember to remove this if we got rid of test_extract_cie_info dependancy, which is preferable

#ifdef __cplusplus
extern "C" {
#endif

/* Data types*/
struct test_Unwind_FrameState_t;
typedef struct test_Unwind_FrameState_t test_Unwind_FrameState;

/* Routines */
void
add_lsda(const unsigned char *fde, struct test_Unwind_Context *context);

inline test_Unwind_Word
test_Unwind_IsSignalFrame (struct test_Unwind_Context *context);

/* Decode DWARF 2 call frame information. Takes pointers the
   instruction sequence to decode, current register information and
   CIE info, and the PC range to evaluate.  */

void
test_execute_cfa_program (const unsigned char *insn_ptr,
		     const unsigned char *insn_end,
		     struct test_Unwind_Context *context,
		     test_Unwind_FrameState *fs);

test_Unwind_Reason_Code
test_uw_frame_state_for(struct test_Unwind_Context *context, test_Unwind_FrameState *fs);

#ifdef __cplusplus
}
#endif

#endif /* TEST_UNWIND_EH */