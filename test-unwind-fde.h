#ifndef TEST_UNWIND_FDE
#define TEST_UNWIND_FDE

#include "test-unwind.h"
#include "dwarf-reg-map-x86_64.h"

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

typedef struct test_Unwind_FrameState_t {
    /* Each register save state can be described in terms of a CFA slot,
        another register, or a location expression.  */
    struct frame_state_reg_info {
        struct {
            union {
                test_Unwind_Word reg;
                test_Unwind_Sword offset;
                const unsigned char *exp;
            } loc;
            enum {
                REG_UNSAVED,
                REG_SAVED_OFFSET,
                REG_SAVED_REG,
                REG_SAVED_EXP,
                REG_SAVED_VAL_OFFSET,
                REG_SAVED_VAL_EXP,
                REG_UNDEFINED
            } how;
        } reg[_DWARF_FRAME_REGISTERS];

        /* Used to implement DW_CFA_remember_state.  */
        struct frame_state_reg_info *prev;

        /* The CFA can be described in terms of a reg+offset or a
            location expression.  */
        test_Unwind_Sword cfa_offset;
        test_Unwind_Word cfa_reg;
        const unsigned char *cfa_exp;
        enum {
            CFA_UNSET,
            CFA_REG_OFFSET,
            CFA_EXP
        } cfa_how;
    } regs;

    /* The PC described by the current frame state.  */
    void *pc;

    /* The information we care about from the CIE/FDE.  */
    test_Unwind_Personality_Fn personality;
    test_Unwind_Sword data_align;
    test_Unwind_Word code_align;
    test_Unwind_Word retaddr_column;
    unsigned char fde_encoding;
    unsigned char lsda_encoding;
    unsigned char saw_z;
    unsigned char signal_frame;
    void *eh_ptr;
}test_Unwind_FrameState;

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

inline const test_fde *
test_next_fde (const test_fde *f);

#ifdef __cplusplus
}
#endif

#endif /* TEST_UNWIND_FDE */