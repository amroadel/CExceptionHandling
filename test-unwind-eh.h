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


struct dwarf_bases {
  void *tbase;
  void *dbase;
  void *func;
} eh_bases; // TODO: add it to the eh_frame_hdr and add the definition to a seperate dwarf library

/* Routines */
void
init_eh_frame_hdr(const unsigned char *eh_frame, const unsigned char *text);

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