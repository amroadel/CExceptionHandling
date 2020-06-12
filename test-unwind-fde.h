#ifndef TEST_UNWIND_FDE
#define TEST_UNWIND_FDE

#include "test-unwind.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int test_uword;
typedef int test_sword;
typedef unsigned char test_ubyte;

/* Data types*/
struct test_dwarf_eh_bases {
    void *tbase;
    void *dbase;
    void *func;
};

struct eh_frame_hdr {
    const unsigned char *self;
    unsigned char version;
    unsigned char eh_frame_encoding;
    unsigned char count_encoding;
    unsigned char entry_encoding;
    const unsigned char *eh_frame;
    int count;
    struct test_dwarf_eh_bases eh_bases;
    const unsigned char *entries;
} header;

struct test_dwarf_cie {
    test_uword length;
    test_sword CIE_id;
    test_ubyte version;
    const unsigned char *augmentation;
}__attribute__((packed, aligned (__alignof__ (void *))));

/* The first few fields of an FDE.  */
struct test_dwarf_fde {
    test_uword length;
    test_sword CIE_delta;
    const unsigned char *pc_begin;
}__attribute__((packed, aligned (__alignof__ (void *))));
typedef struct test_dwarf_fde test_fde;

/* Routines */
void
init_eh_frame_hdr(const unsigned char *eh_frame, const unsigned char *text);

inline const struct test_dwarf_cie *
test_get_cie(const struct test_dwarf_fde *fde);

unsigned char
test_get_cie_encoding (const struct test_dwarf_cie *cie);

inline unsigned char
test_get_fde_encoding (const struct test_dwarf_fde *fde);

const test_fde *
find_fde(void *pc, struct test_dwarf_eh_bases *bases);

#ifdef __cplusplus
}
#endif

#endif /* TEST_UNWIND_FDE */