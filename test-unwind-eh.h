#ifndef TEST_UNWIND_EH
#define TEST_UNWIND_EH

#include "test-unwind.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Data types*/
struct eh_frame_hdr;
struct test_Unwind_FrameState;
typedef struct test_Unwind_FrameState test_Unwind_FrameState;

struct eh_bases {
  void *tbase;
  void *dbase;
  void *func;
};

/* Routines */
void
init_eh_frame_hdr(const unsigned char *eh_frame);

const unsigned char *
find_fde(void *ra);
/*void
fill_context(const unsigned char * fde, struct test_Unwind_Context *context);*/

void
add_lsda(const unsigned char *fde, struct test_Unwind_Context *context);

#ifdef __cplusplus
}
#endif

#endif /* TEST_UNWIND_EH */