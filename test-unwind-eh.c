#include "test-unwind-eh.h"
#include "test-unwind-pe.h"
#include "dwarf-reg-map-x86_64.h"
#include "stdlib.h"
#include "stdio.h" // remember to delete this

#ifdef __cplusplus
extern "C" {
#endif

/* Data types*/
struct eh_frame_hdr {
    const unsigned char *self;
    unsigned char version;
    unsigned char eh_frame_encoding;
    unsigned char count_encoding;
    unsigned char entry_encoding;
    const unsigned char *eh_frame;
    int count;
    const unsigned char *entries;
} header;

struct test_Unwind_FrameState
{
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
};

struct test_Unwind_Context {
    void *cfa;
    void *ra;
    void *lsda;
    struct dwarf_bases bases;
    // keep them for now until we know more about them
    test_Unwind_Word flags;
    test_Unwind_Word version;
    test_Unwind_Word args_size;
};

/* Routines */
void
init_eh_frame_hdr(const unsigned char *eh_frame_hdr, const unsigned char *text)
{
    const unsigned char *p = eh_frame_hdr;
    header.self = p;
    header.version = *p++;
    header.eh_frame_encoding = *p++;
    header.count_encoding = *p++;
    header.entry_encoding = *p++;

    test_Unwind_Ptr val;

    if (header.eh_frame_encoding != DW_EH_PE_omit) {
        p = read_encoded_value_with_base(header.eh_frame_encoding,
        (test_Unwind_Ptr)eh_frame_hdr, p, &val);
        header.eh_frame = (const unsigned char *)val;
    } else {
        header.eh_frame = NULL;
    }
    
    if (header.count_encoding != DW_EH_PE_omit) {
        p = read_encoded_value_with_base(header.count_encoding,
        (test_Unwind_Ptr)eh_frame_hdr, p, &val);
        header.count = (int)val;
    } else {
        header.count = 0;
    }

    if (header.entry_encoding != DW_EH_PE_omit)
        header.entries = p;
    else
        header.entries = NULL;

    eh_bases.tbase = (void *)text;
    eh_bases.dbase = (void *)eh_frame_hdr;
    eh_bases.func = NULL;
}

const unsigned char *
find_fde(void *ra) // TODO: add the bases
{
    if (header.entries == NULL || header.count == 0)
        abort();
    const unsigned char *p = header.entries;

    test_Unwind_Ptr ip = (test_Unwind_Ptr)ra;
    test_Unwind_Ptr base;
    test_Unwind_Ptr fde;

    p = read_encoded_value_with_base(header.entry_encoding,
    (test_Unwind_Ptr)header.self, p, &base);
    for (int i = 0; i < header.count; i++) {
        printf(" base is %lx\n", base);
        p = read_encoded_value_with_base(header.entry_encoding,
        (test_Unwind_Ptr)header.self, p, &fde);

        if (ip > base) {
            if (i + 1 == header.count) {
                printf(" case 1 base is %lx\n", base);
                printf(" ra is %lx\n", ip);
                return (const unsigned char *)fde;
            }
            p = read_encoded_value_with_base(header.entry_encoding,
            (test_Unwind_Ptr)header.self, p, &base);
            if (base > ip) {
                printf(" case 2 base is %lx\n", base);
                printf(" ra is %lx\n", ip);
                return (const unsigned char *)fde;
            }
        } else {
            if (i + 1 == header.count)
                abort();
            p = read_encoded_value_with_base(header.entry_encoding,
            (test_Unwind_Ptr)header.self, p, &base);
        }            
    }
    abort();
}

void
fill_context(const unsigned char *fde, struct test_Unwind_Context *context);

#ifdef __cplusplus
}
#endif