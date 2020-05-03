#ifndef TEST_UNWIND_EH
#define TEST_UNWIND_EH

#include "test-unwind.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Data types*/
struct eh_frame_hdr;

/* Routines */
static void
init_eh_frame_hdr(const unsigned char *eh_frame);

static const unsigned char *
find_fde(void *ra);

static void
fill_context(const unsigned char *fde, test_Unwind_Context *context);

#ifdef __cplusplus
}
#endif

#endif /* TEST_UNWIND_EH */