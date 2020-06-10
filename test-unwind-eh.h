#ifndef TEST_UNWIND_EH
#define TEST_UNWIND_EH

#include "test-unwind.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Data types*/
struct eh_frame_hdr;
struct test_Unwind_FrameState;
struct test_dwarf_cie;
typedef struct test_Unwind_FrameState test_Unwind_FrameState;
typedef struct test_dwarf_fde test_fde; 

typedef unsigned int test_uword;
typedef int test_sword;
typedef unsigned char test_ubyte;


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

void
add_lsda(const unsigned char *fde, struct test_Unwind_Context *context);

static inline const struct test_dwarf_cie *
test_get_cie (const struct test_dwarf_fde *f);

static const unsigned char *
test_extract_cie_info (const struct test_dwarf_cie *cie, struct test_Unwind_Context *context,
		  test_Unwind_FrameState *fs);

static test_Unwind_Reason_Code
test_uw_frame_state_for (struct test_Unwind_Context *context, test_Unwind_FrameState *fs);

#ifdef __cplusplus
}
#endif

#endif /* TEST_UNWIND_EH */