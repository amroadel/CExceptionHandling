#ifndef TEST_UNWIND_EH
#define TEST_UNWIND_EH

#include "test-unwind.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Routines */
static test_Unwind_Ptr
find_fde(const unsigned char *eh_frame_hdr);

void
fill_context(test_Unwind_Ptr fde, test_Unwind_Context *context);

#ifdef __cplusplus
}
#endif

#endif /* TEST_UNWIND_EH */