#include "test-unwind-eh.h"
#include "test-unwind-pe.h"
#include "test-unwind-fde.h"
#include "dwarf-reg-map-x86_64.h"
#include "stdlib.h"
#include "stdio.h" // remember to delete this
#include "string.h" // for memset and memcpy 

#ifdef __cplusplus
extern "C" {
#endif

#define STACK_GROWS_DOWNWARD 0  // TODO: This needs more resreach

/* Data types*/
typedef struct test_Unwind_FrameState_t {
    /*  Each register save state can be described in terms of a CFA slot,
        another register, or a location expression  */
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

        /*  The CFA can be described in terms of a reg+offset or a
            location expression  */
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
} test_Unwind_FrameState;

struct test_Unwind_Context {
    test_Unwind_Context_Reg_Val reg[_DWARF_FRAME_REGISTERS];
    void *cfa;
    void *ra;
    void *lsda;
    struct test_dwarf_eh_bases bases;
    
    #define SIGNAL_FRAME_BIT ((~(test_Unwind_Word)0 >> 1) + 1)
    #define EXTENDED_CONTEXT_BIT ((~(test_Unwind_Word)0 >> 2) + 1)
    test_Unwind_Word flags;
    test_Unwind_Word version;
    test_Unwind_Word args_size;
    char by_value[_DWARF_FRAME_REGISTERS];
};

unsigned char dwarf_reg_size_table[_DWARF_FRAME_REGISTERS];

/* Routines */
/* Unwind support functions */
inline test_Unwind_Word
test_Unwind_Get_Unwind_Word(test_Unwind_Context_Reg_Val val)
{
    return (test_Unwind_Word)(test_Unwind_Internal_Ptr)val;
}

inline test_Unwind_Context_Reg_Val
test_Unwind_Get_Unwind_Context_Reg_Val(test_Unwind_Word val)
{
    return (test_Unwind_Context_Reg_Val)(test_Unwind_Internal_Ptr)val;
}

#ifndef ASSUME_EXTENDED_UNWIND_CONTEXT
#define ASSUME_EXTENDED_UNWIND_CONTEXT 0
#endif

inline test_Unwind_Word
test_Unwind_IsSignalFrame(struct test_Unwind_Context *context)
{
    return (context->flags & SIGNAL_FRAME_BIT) ? 1 : 0;
}

inline void
test_Unwind_SetSignalFrame(struct test_Unwind_Context *context, int val)
{
    if (val)
        context->flags |= SIGNAL_FRAME_BIT;
    else
        context->flags &= ~SIGNAL_FRAME_BIT;
}

inline test_Unwind_Word
test_Unwind_IsExtendedContext(struct test_Unwind_Context *context)
{
    return (ASSUME_EXTENDED_UNWIND_CONTEXT
        || (context->flags & EXTENDED_CONTEXT_BIT));
}

/* Unwind setters and getters */
test_Unwind_Word
test_Unwind_GetGR(struct test_Unwind_Context *context, int index)
{
    int size, index;
    test_Unwind_Context_Reg_Val val;

    #ifdef DWARF_ZERO_REG
    if (regno == DWARF_ZERO_REG)
        return 0;
    #endif

    index = _DWARF_REG_TO_UNWIND_COLUMN(index);
    size = dwarf_reg_size_table[index];
    val = context->reg[index];

    if (test_Unwind_IsExtendedContext(context) && context->by_value[index])
        return _Unwind_Get_Unwind_Word (val);

    /* Special Handling: aarch64 needs modification for lazy register values */

    if (size == sizeof(test_Unwind_Ptr))
        return *(test_Unwind_Ptr *)(test_Unwind_Internal_Ptr)val;
    else
        return *(test_Unwind_Word *)(test_Unwind_Internal_Ptr)val;
}

void
test_Unwind_SetGR(struct test_Unwind_Context *context, int index, test_Unwind_Word val)
{
    int size;
    void *p;

    index = _DWARF_REG_TO_UNWIND_COLUMN(index);
    size = dwarf_reg_size_table[index];

    if (test_Unwind_IsExtendedContext(context) && context->by_value[index]) {
        context->reg[index] = test_Unwind_Get_Unwind_Context_Reg_Val (val);
        return;
    }

    p = (void *)(test_Unwind_Internal_Ptr)context->reg[index];
    if (size == sizeof(test_Unwind_Ptr))
        *(test_Unwind_Ptr *)p = val;
    else
        *(test_Unwind_Word *)p = val;
}

test_Unwind_Ptr
test_Unwind_GetIP(struct test_Unwind_Context *context)
{
    return (test_Unwind_Ptr) context->ra;
}

void
test_Unwind_SetIP(struct test_Unwind_Context *context, test_Unwind_Ptr val)
{
    context->ra = (void *)val;
}

test_Unwind_Word
test_Unwind_GetCFA(struct test_Unwind_Context *context)
{
    return (test_Unwind_Word) context->cfa;
}

test_Unwind_Ptr
test_Unwind_GetLanguageSpecificData(struct test_Unwind_Context *context)
{
    return (test_Unwind_Ptr) context->lsda;
}

test_Unwind_Ptr
test_Unwind_GetRegionStart(struct test_Unwind_Context *context)
{
    return (test_Unwind_Ptr) context->bases.func;
}

test_Unwind_Ptr
test_Unwind_GetTextRelBase(struct test_Unwind_Context *context)
{
    return (test_Unwind_Ptr) context->bases.tbase;
}

test_Unwind_Ptr
test_Unwind_GetDataRelBase(struct test_Unwind_Context *context)
{
    return (test_Unwind_Ptr) context->bases.dbase;
}

/* Gerneral Register management */
inline void *
test_Unwind_GetPtr(struct test_Unwind_Context *context, int index)
{
    return (void *)(test_Unwind_Internal_Ptr)test_Unwind_GetGR(context, index);
}

inline void *
test_Unwind_GetGRPtr(struct test_Unwind_Context *context, int index)
{
    index = _DWARF_REG_TO_UNWIND_COLUMN(index);
    if (test_Unwind_IsExtendedContext(context) && context->by_value[index])
        return &context->reg[index];
    return (void *)(test_Unwind_Internal_Ptr)context->reg[index];
}

inline void
test_Unwind_SetGRPtr(struct test_Unwind_Context *context, int index, void *p)
{
    index = _DWARF_REG_TO_UNWIND_COLUMN(index);
    if (test_Unwind_IsExtendedContext(context))
        context->by_value[index] = 0;
    context->reg[index] = (test_Unwind_Context_Reg_Val)(test_Unwind_Internal_Ptr)p;
}

inline void
test_Unwind_SetGRValue(struct test_Unwind_Context *context, int index, test_Unwind_Word val)
{
    index = _DWARF_REG_TO_UNWIND_COLUMN(index);
    context->by_value[index] = 1;
    context->reg[index] = test_Unwind_Get_Unwind_Context_Reg_Val(val);
}

inline char
test_Unwind_GRByValue(struct test_Unwind_Context *context, int index)
{
    index = _DWARF_REG_TO_UNWIND_COLUMN(index);
    return context->by_value[index];
}

/* Dwarf Interpreter */
void
test_execute_cfa_program(const unsigned char *insn_ptr, const unsigned char *insn_end,
    struct test_Unwind_Context *context, test_Unwind_FrameState *fs)
{

}

test_Unwind_Word
test_execute_stack_op(const unsigned char *op_ptr, const unsigned char *op_end,
    struct test_Unwind_Context *context, test_Unwind_Word initial)
{

}

/* Context management */
void
_update_context(struct test_Unwind_Context *context, test_Unwind_FrameState *fs)
{
    struct test_Unwind_Context orig_context = *context;
    void *cfa;
    long i;

    /* Special Handling: check gcc equivelant */

    /* Compute the CFA */
    switch (fs->regs.cfa_how) {
    case CFA_REG_OFFSET:
        cfa = test_Unwind_GetPtr(&orig_context, fs->regs.cfa_reg);
        cfa += fs->regs.cfa_offset;
        break;
    case CFA_EXP: {
        const unsigned char *exp = fs->regs.cfa_exp;
        _uleb128_t len;
        exp = read_uleb128(exp, &len);
        cfa = (void *)(test_Unwind_Ptr)test_execute_stack_op(exp, exp + len, &orig_context, 0);
        break;
    }
    default:
        abort();
    }
    context->cfa = cfa;

    /* Compute all registers */
    for (i = 0; i < _DWARF_FRAME_REGISTERS + 1; ++i)
        switch (fs->regs.reg[i].how) {
        case REG_UNSAVED:
        case REG_UNDEFINED:
            break;
        case REG_SAVED_REG:
            if (test_Unwind_GRByValue(&orig_context, fs->regs.reg[i].loc.reg))
                test_Unwind_SetGRValue(context, i,
                    test_Unwind_GetGR(&orig_context, fs->regs.reg[i].loc.reg));
            else
                test_Unwind_SetGRPtr(context, i,
                    test_Unwind_GetGRPtr(&orig_context, fs->regs.reg[i].loc.reg));
            break;
        case REG_SAVED_OFFSET:
            test_Unwind_SetGRPtr(context, i, (void *)(cfa + fs->regs.reg[i].loc.offset));
            break;
        case REG_SAVED_EXP: {
            const unsigned char *exp = fs->regs.reg[i].loc.exp;
            _uleb128_t len;
            test_Unwind_Ptr val;
            exp = read_uleb128(exp, &len);
            val = test_execute_stack_op(exp, exp + len, &orig_context, (test_Unwind_Ptr)cfa);
            test_Unwind_SetGRPtr(context, i, (void *)val);
            break;
        }
        case REG_SAVED_VAL_OFFSET:
            test_Unwind_SetGRValue(context, i,
                (test_Unwind_Word)(test_Unwind_Internal_Ptr)(cfa + fs->regs.reg[i].loc.offset));
            break;
        case REG_SAVED_VAL_EXP: {
            const unsigned char *exp = fs->regs.reg[i].loc.exp;
            _uleb128_t len;
            test_Unwind_Ptr val;
            exp = read_uleb128(exp, &len);
            val = test_execute_stack_op(exp, exp + len, &orig_context, (test_Unwind_Ptr) cfa);
            test_Unwind_SetGRValue(context, i, val);
            break;
        }
        }

    test_Unwind_SetSignalFrame (context, fs->signal_frame);

    #ifdef MD_FROB_UPDATE_CONTEXT //TODO: need to look more into when this is true
    /* checking for sigreturn() */
    if ((pc[0] == 0x38007777 || pc[0] == 0x38000077
        || pc[0] == 0x38006666 || pc[0] == 0x380000AC)
        && pc[1] == 0x44000002)
        test_Unwind_SetSignalFrame (context, 1);
    #endif
}

void __attribute__((noinline))
_init_context(struct test_Unwind_Context *context, void *outer_cfa, void *outer_ra)
{
    void *ra = __builtin_extract_return_addr(__builtin_return_address(0));
    test_Unwind_FrameState fs;

    context = (struct test_Unwind_Context *)malloc(sizeof(struct test_Unwind_Context));
    memset(context, 0, sizeof(struct test_Unwind_Context));
    context->ra = ra;
    if (!ASSUME_EXTENDED_UNWIND_CONTEXT)
        context->flags |= EXTENDED_CONTEXT_BIT;

    test_uw_frame_state_for(context, &fs);

    if (dwarf_reg_size_table[0] == 0)
        _builtin_init_dwarf_reg_size_table(dwarf_reg_size_table);

    /* Force the frame state to use the known cfa value.  */ //TODO: we should check why??
    if (dwarf_reg_size_table[_builtin_dwarf_sp_column()] == sizeof(test_Unwind_Ptr)) {
        test_Unwind_Ptr sp = (test_Unwind_Internal_Ptr)outer_cfa;
        test_Unwind_SetGRPtr(context, _builtin_dwarf_sp_column (), &sp);
    } else {
        test_Unwind_Word sp = (test_Unwind_Internal_Ptr)outer_cfa;
        test_Unwind_SetGRPtr(context, _builtin_dwarf_sp_column (), &sp);
    }
    fs.regs.cfa_how = CFA_REG_OFFSET;
    fs.regs.cfa_reg = _builtin_dwarf_sp_column();
    fs.regs.cfa_offset = 0;

    _update_context(context, &fs);

    context->ra = __builtin_extract_return_addr(outer_ra);
    /* Special Handling: aarch64 needs modification for return address value */
}

void
test_uw_update_context(struct test_Unwind_Context *context, test_Unwind_FrameState *fs)
{
    _update_context(context, fs);

    if (fs->regs.reg[_DWARF_REG_TO_UNWIND_COLUMN(fs->retaddr_column)].how == REG_UNDEFINED)
        context->ra = 0;    /* outermost stack frame */
    else
        context->ra = __builtin_extract_return_addr(test_Unwind_GetPtr(context, fs->retaddr_column));
    /* Special Handling: aarch64 needs modification for return address value */
}

inline test_Unwind_Ptr
test_uw_identify_context(struct test_Unwind_Context *context)
{
    // TODO: This needs more resreach
    if (STACK_GROWS_DOWNWARD)
        return test_Unwind_GetCFA(context) - test_Unwind_IsSignalFrame (context);
    else
        return test_Unwind_GetCFA(context) + test_Unwind_IsSignalFrame (context);
}

inline void
uw_copy_context(struct test_Unwind_Context *target, struct test_Unwind_Context *source)
{
    target = (struct test_Unwind_Context *)malloc(sizeof(struct test_Unwind_Context));
    memcpy(target, source, sizeof(struct test_Unwind_Context));
}

const unsigned char *
test_extract_cie_info (const struct test_dwarf_cie *cie, struct test_Unwind_Context *context,
    test_Unwind_FrameState *fs)
{

    const unsigned char *aug = cie->augmentation;
    const unsigned char *p = aug + strlen ((const char *)aug) + 1;
    const unsigned char *ret = NULL;
    _uleb128_t utmp;
    _sleb128_t stmp;

    /* g++ v2 "eh" has pointer immediately following augmentation string,
        so it must be handled first.  */
    if (aug[0] == 'e' && aug[1] == 'h')
    {
        //fs->eh_ptr = read_pointer (p); //TODO: Check for read_pointer()
        fs->eh_ptr = (void *)p;
        p += sizeof (void *);
        aug += 2;
    }

  /* After the augmentation resp. pointer for "eh" augmentation
     follows for CIE version >= 4 address size byte and
     segment size byte.  */
  if (cie->version >= 4)
    {
      if (p[0] != sizeof (void *) || p[1] != 0)
	    return NULL;
      p += 2;
    }
    /* Immediately following this are the code and
        data alignment and return address column.  */
    p = read_uleb128 (p, &utmp);
    fs->code_align = (test_Unwind_Word)utmp;
    p = read_sleb128 (p, &stmp);
    fs->data_align = (test_Unwind_Sword)stmp;
    if (cie->version == 1)
        fs->retaddr_column = *p++;
    else
        {
        p = read_uleb128 (p, &utmp);
        fs->retaddr_column = (test_Unwind_Word)utmp;
        }
    fs->lsda_encoding = DW_EH_PE_omit;

    /* If the augmentation starts with 'z', then a uleb128 immediately
        follows containing the length of the augmentation field following
        the size.  */
    if (*aug == 'z')
        {
        p = read_uleb128 (p, &utmp);
        ret = p + utmp;

        fs->saw_z = 1;
        ++aug;
        }

    /* Iterate over recognized augmentation subsequences.  */
    while (*aug != '\0')
        {
        /* "L" indicates a byte showing how the LSDA pointer is encoded.  */
        if (aug[0] == 'L')
        {
        fs->lsda_encoding = *p++;
        aug += 1;
        }

        /* "R" indicates a byte indicating how FDE addresses are encoded.  */
        else if (aug[0] == 'R')
        {
        fs->fde_encoding = *p++;
        aug += 1;
        }

        /* "P" indicates a personality routine in the CIE augmentation.  */
        else if (aug[0] == 'P')
        {
        test_Unwind_Ptr personality;

        p = read_encoded_value (context, *p, p + 1, &personality);
        fs->personality = (test_Unwind_Personality_Fn) personality;
        aug += 1;
        }

        /* "S" indicates a signal frame.  */
        else if (aug[0] == 'S')
        {
        fs->signal_frame = 1;
        aug += 1;
        }
        /* aarch64 B-key pointer authentication.  */
        else if (aug[0] == 'B')
        {
        aug += 1;
        }

        /* Otherwise we have an unknown augmentation string.
        Bail unless we saw a 'z' prefix.  */
        else
        return ret;
        }

    return ret ? ret : p;
}

test_Unwind_Reason_Code
test_uw_frame_state_for (struct test_Unwind_Context *context, test_Unwind_FrameState *fs)
{
    const struct test_dwarf_fde *fde;
    const struct test_dwarf_cie *cie;
    const unsigned char *aug, *insn, *end;

    fs = (test_Unwind_FrameState *)malloc(sizeof(test_Unwind_FrameState));
    memset (fs, 0, sizeof(test_Unwind_FrameState));
    context->args_size = 0;
    context->lsda = 0;

    if (context->ra == 0)
        return _URC_END_OF_STACK;
    
    fde = find_fde(context->ra, &context->bases);

    if (fde == NULL)
        return _URC_END_OF_STACK;

    fs->pc = context->bases.func;
    cie = test_get_cie (fde);
    insn = test_extract_cie_info (cie, context, fs);
    if (insn == NULL)
        /* CIE contained unknown augmentation.  */
        return _URC_FATAL_PHASE1_ERROR;

    /* First decode all the insns in the CIE.  */
    end = (const unsigned char *) test_next_fde ((const struct test_dwarf_fde *) cie);
    //test_execute_cfa_program (insn, end, context, fs);

    /* Locate augmentation for the fde.  */
    aug = (const unsigned char *) fde + sizeof (*fde);
    aug += 2 * size_of_encoded_value (fs->fde_encoding);
    insn = NULL;

    if (fs->saw_z)
    {
        _uleb128_t i;
        aug = read_uleb128 (aug, &i);
        insn = aug + i;
    }

    if (fs->lsda_encoding != DW_EH_PE_omit)
    {
        test_Unwind_Ptr lsda;

        aug = read_encoded_value (context, fs->lsda_encoding, aug, &lsda);
        context->lsda = (void *) lsda;
        printf("generated lsda: %p\n", lsda);
    }

    /* Then the insns in the FDE up to our target PC.  */
    if (insn == NULL)
        insn = aug;
    end = (const unsigned char *) test_next_fde (fde);
    //test_execute_cfa_program (insn, end, context, fs);

    return _URC_NO_REASON;

}

inline test_Unwind_Personality_Fn
uw_get_personality(test_Unwind_FrameState *fs)
{
    return fs->personality;
}

#ifdef __cplusplus
}
#endif