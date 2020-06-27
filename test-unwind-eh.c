#include "test-unwind-eh.h"
#include "test-unwind-pe.h"
#include "test-unwind-fde.h"
#include "test-unwind-cfi.h"
#include "dwarf-reg-map-x86_64.h"
#include "stdlib.h"
#include "stdio.h" //TODO: remember to delete this
#include "string.h" //TODO: for memset and memcpy 

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __STACK_GROWS_DOWNWARD__    // TODO: This needs more resreach
#define __STACK_GROWS_DOWNWARD__ 1
#endif

#define _UNWIND_COLUMN_IN_RANGE(x) ((x) <= _DWARF_FRAME_REGISTERS)

/*  Data types*/
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

    /*  Used to implement DW_CFA_remember_state.  */
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
};

typedef struct test_Unwind_FrameState_t {
    /*  The PC described by the current frame state.  */
    void *pc;
    struct frame_state_reg_info regs;
    /*  The information we care about from the CIE/FDE.  */
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

typedef union {
    test_Unwind_Ptr ptr;
    test_Unwind_Word word;
} test_Unwind_SpTmp;

unsigned char dwarf_reg_size_table[_DWARF_FRAME_REGISTERS];

/*  Routines  */
/*  Unwind support functions  */
static inline test_Unwind_Word
test_Unwind_Get_Unwind_Word(test_Unwind_Context_Reg_Val val)
{
    return (test_Unwind_Word)(test_Unwind_Internal_Ptr)val;
}

static inline test_Unwind_Context_Reg_Val
test_Unwind_Get_Unwind_Context_Reg_Val(test_Unwind_Word val)
{
    return (test_Unwind_Context_Reg_Val)(test_Unwind_Internal_Ptr)val;
}

#ifndef ASSUME_EXTENDED_UNWIND_CONTEXT
#define ASSUME_EXTENDED_UNWIND_CONTEXT 0
#endif

static inline test_Unwind_Word
test_Unwind_IsSignalFrame(struct test_Unwind_Context *context)
{
    return (context->flags & SIGNAL_FRAME_BIT) ? 1 : 0;
}

static inline void
test_Unwind_SetSignalFrame(struct test_Unwind_Context *context, int val)
{
    if (val)
        context->flags |= SIGNAL_FRAME_BIT;
    else
        context->flags &= ~SIGNAL_FRAME_BIT;
}

static inline test_Unwind_Word
test_Unwind_IsExtendedContext(struct test_Unwind_Context *context)
{
    return (ASSUME_EXTENDED_UNWIND_CONTEXT
        || (context->flags & EXTENDED_CONTEXT_BIT));
}

/*  This function is called during unwinding. It is intended as a hook
   for a debugger to intercept exceptions. CFA is the CFA of the
   target frame. HANDLER is the PC to which control will be
   transferred. http://agentzh.org/misc/code/systemtap/includes/sys/sdt.h.html  */

void
test_Unwind_DebugHook (void *cfa __attribute__ ((__unused__)),
    void *handler __attribute__ ((__unused__)))
{
    /*  We only want to use stap probes starting with v3. Earlier
        versions added too much startup cost.  */
    #if defined (HAVE_SYS_SDT_H) && defined (STAP_PROBE2) && _SDT_NOTE_TYPE >= 3
    STAP_PROBE2 (libgcc, unwind, cfa, handler);
    #else
    asm ("");
    #endif
}

/*  Unwind setters and getters  */
test_Unwind_Word
test_Unwind_GetGR(struct test_Unwind_Context *context, int index)
{
    int size;
    test_Unwind_Context_Reg_Val val;

    #ifdef DWARF_ZERO_REG
    if (regno == DWARF_ZERO_REG)
        return 0;
    #endif

    index = _DWARF_REG_TO_UNWIND_COLUMN(index);
    size = dwarf_reg_size_table[index];
    val = context->reg[index];

    if (test_Unwind_IsExtendedContext(context) && context->by_value[index])
        return test_Unwind_Get_Unwind_Word (val);

    /*  Special Handling: aarch64 needs modification for lazy register values  */

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
    return (test_Unwind_Ptr)context->ra;
}

void
test_Unwind_SetIP(struct test_Unwind_Context *context, test_Unwind_Ptr val)
{
    context->ra = (void *)val;
}

test_Unwind_Word
test_Unwind_GetCFA(struct test_Unwind_Context *context)
{
    return (test_Unwind_Word)context->cfa;
}

test_Unwind_Ptr
test_Unwind_GetLanguageSpecificData(struct test_Unwind_Context *context)
{
    return (test_Unwind_Ptr)context->lsda;
}

test_Unwind_Ptr
test_Unwind_GetRegionStart(struct test_Unwind_Context *context)
{
    return (test_Unwind_Ptr)context->bases.func;
}

test_Unwind_Ptr
test_Unwind_GetTextRelBase(struct test_Unwind_Context *context)
{
    return (test_Unwind_Ptr)context->bases.tbase;
}

test_Unwind_Ptr
test_Unwind_GetDataRelBase(struct test_Unwind_Context *context)
{
    return (test_Unwind_Ptr)context->bases.dbase;
}

/*  Gerneral Register management  */
static inline void *
test_Unwind_GetPtr(struct test_Unwind_Context *context, int index)
{
    return (void *)(test_Unwind_Internal_Ptr)test_Unwind_GetGR(context, index);
}

static inline void *
test_Unwind_GetGRPtr(struct test_Unwind_Context *context, int index)
{
    index = _DWARF_REG_TO_UNWIND_COLUMN(index);
    if (test_Unwind_IsExtendedContext(context) && context->by_value[index])
        return &context->reg[index];
    return (void *)(test_Unwind_Internal_Ptr)context->reg[index];
}

static inline void
test_Unwind_SetGRPtr(struct test_Unwind_Context *context, int index, void *p)
{
    index = _DWARF_REG_TO_UNWIND_COLUMN(index);
    if (test_Unwind_IsExtendedContext(context))
        context->by_value[index] = 0;
    context->reg[index] = (test_Unwind_Context_Reg_Val)(test_Unwind_Internal_Ptr)p;
}

static inline void
test_Unwind_SetGRValue(struct test_Unwind_Context *context, int index, test_Unwind_Word val)
{
    index = _DWARF_REG_TO_UNWIND_COLUMN(index);
    context->by_value[index] = 1;
    context->reg[index] = test_Unwind_Get_Unwind_Context_Reg_Val(val);
}

static inline char
test_Unwind_GRByValue(struct test_Unwind_Context *context, int index)
{
    index = _DWARF_REG_TO_UNWIND_COLUMN(index);
    return context->by_value[index];
}

static inline void
test_Unwind_SetSpColumn(struct test_Unwind_Context *context, void *cfa,
	test_Unwind_SpTmp *tmp_sp)
{
    int size = dwarf_reg_size_table[_builtin_dwarf_sp_column()];
    if (size == sizeof(test_Unwind_Ptr))
        tmp_sp->ptr = (test_Unwind_Ptr) cfa;
    else
        tmp_sp->word = (test_Unwind_Ptr) cfa;
    test_Unwind_SetGRPtr(context, _builtin_dwarf_sp_column(), tmp_sp);
}

/*  Dwarf Interpreter  */
union unaligned
{
  void *p;
  unsigned u2 __attribute__ ((mode (HI)));
  unsigned u4 __attribute__ ((mode (SI)));
  unsigned u8 __attribute__ ((mode (DI)));
  signed s2 __attribute__ ((mode (HI)));
  signed s4 __attribute__ ((mode (SI)));
  signed s8 __attribute__ ((mode (DI)));
} __attribute__((packed));

static inline void *
read_pointer (const void *p) { const union unaligned *up = p; return up->p; }

static inline int
read_1u (const void *p) { return *(const unsigned char *) p; }

static inline int
read_1s (const void *p) { return *(const signed char *) p; }

static inline int
read_2u (const void *p) { const union unaligned *up = p; return up->u2; }

static inline int
read_2s (const void *p) { const union unaligned *up = p; return up->s2; }

static inline unsigned int
read_4u (const void *p) { const union unaligned *up = p; return up->u4; }

static inline int
read_4s (const void *p) { const union unaligned *up = p; return up->s4; }

static inline unsigned long
read_8u (const void *p) { const union unaligned *up = p; return up->u8; }

static inline unsigned long
read_8s (const void *p) { const union unaligned *up = p; return up->s8; }

void
test_execute_cfa_program(const unsigned char *insn_ptr, const unsigned char *insn_end,
    struct test_Unwind_Context *context, test_Unwind_FrameState *fs)
{
    struct frame_state_reg_info *unused_rs = NULL;

    /* Don't allow remember/restore between CIE and FDE programs.  */
    fs->regs.prev = NULL;//<<>> only used for stacking the set of reg rules for a frame

    /* The comparison with the return address uses < rather than <= because
        we are only interested in the effects of code **before the call**; for a
        noreturn function, the return address may point to unrelated code with
        a different **stack configuration** that we are not interested in.  We
        assume that the call itself is **unwind info-neutral**; if not, or if
        there are delay instructions that adjust the stack, these must be
        reflected at the point immediately before the call insn.
        **In signal frames**, return address is after last completed instruction,
        so we add 1 to return address to make the comparison <=.  */
    while (insn_ptr < insn_end
        && fs->pc < context->ra + test_Unwind_IsSignalFrame (context))
    {
        unsigned char insn = *insn_ptr++;////to take the highest byte
        _uleb128_t reg, utmp;
        _sleb128_t offset, stmp;

        if ((insn & 0xc0) == DW_CFA_advance_loc)//The required action is to create a 
        //new table row with a location value that is computed by taking the current 
        //entry’s location value and adding the value of delta * code_alignment_factor.
    fs->pc += (insn & 0x3f) * fs->code_align;
        else if ((insn & 0xc0) == DW_CFA_offset)//The required action is to change the rule for the register
        //indicated by the register number to be an offset(N) (CFA+N) rule where the value of N
        //is factored offset * data_alignment_factor.
    {
        reg = insn & 0x3f;
        insn_ptr = read_uleb128 (insn_ptr, &utmp);
        offset = (test_Unwind_Sword) utmp * fs->data_align;
        reg = _DWARF_REG_TO_UNWIND_COLUMN (reg);//<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>
        if (_UNWIND_COLUMN_IN_RANGE (reg))
        {
            fs->regs.reg[reg].how = REG_SAVED_OFFSET;
            fs->regs.reg[reg].loc.offset = offset;
        }
    }
        else if ((insn & 0xc0) == DW_CFA_restore)//The required action is to change
    //the rule for the indicated register to the rule assigned it by the
    //initial_instructions in the CIE.
    {
        reg = insn & 0x3f;
        reg = _DWARF_REG_TO_UNWIND_COLUMN (reg);
        if (_UNWIND_COLUMN_IN_RANGE (reg))
        fs->regs.reg[reg].how = REG_UNSAVED;
    }
        else switch (insn)
    {
    case DW_CFA_set_loc://The required action is to create a new table row using the specified address as the location
        {
        test_Unwind_Ptr pc;//unsigned pointer

        insn_ptr = read_encoded_value (context, fs->fde_encoding,
                        insn_ptr, &pc);///ectract the signal operand and but it in pc
        fs->pc = (void *) pc;
        }
        break;

    case DW_CFA_advance_loc1://This instruction is identical to DW_CFA_advance_loc except 
    //for the encoding and size of the delta operand.
        fs->pc += read_1u (insn_ptr) * fs->code_align;///???
        insn_ptr += 1;
        break;
    case DW_CFA_advance_loc2://This instruction is identical to DW_CFA_advance_loc except 
    //for the encoding and size of the delta operand.
        fs->pc += read_2u (insn_ptr) * fs->code_align;
        insn_ptr += 2;
        break;
    case DW_CFA_advance_loc4://This instruction is identical to DW_CFA_advance_loc except 
    //for the encoding and size of the delta operand.
        fs->pc += read_4u (insn_ptr) * fs->code_align;
        insn_ptr += 4;
        break;

    case DW_CFA_offset_extended://This instruction is identical to DW_CFA_offset except 
    //for the encoding and size of the register operand.
        insn_ptr = read_uleb128 (insn_ptr, &reg);
        insn_ptr = read_uleb128 (insn_ptr, &utmp);
        offset = (test_Unwind_Sword) utmp * fs->data_align;//<<>>casting to fit offset type in unwind
        //frame state
        reg = _DWARF_REG_TO_UNWIND_COLUMN (reg);
        if (_UNWIND_COLUMN_IN_RANGE (reg))
        {
            fs->regs.reg[reg].how = REG_SAVED_OFFSET;
            fs->regs.reg[reg].loc.offset = offset;
        }
        break;

    case DW_CFA_restore_extended://This instruction is identical to DW_CFA_restore except 
    //for the encoding and size of the register operand.
        insn_ptr = read_uleb128 (insn_ptr, &reg);
        /* FIXME, this is wrong; the CIE might have said that the
            register was saved somewhere.  */
        reg = _DWARF_REG_TO_UNWIND_COLUMN (reg);
        if (_UNWIND_COLUMN_IN_RANGE (reg))
        fs->regs.reg[reg].how = REG_UNSAVED;
        break;

    case DW_CFA_same_value://The required action is to set the
    // rule for the specified register to “same value.”
        insn_ptr = read_uleb128 (insn_ptr, &reg);
        reg = _DWARF_REG_TO_UNWIND_COLUMN (reg);
        if (_UNWIND_COLUMN_IN_RANGE (reg))
        fs->regs.reg[reg].how = REG_UNSAVED;
        break;

    case DW_CFA_undefined://The required action is to set the rule for the
    //specified register to “undefined.”
        insn_ptr = read_uleb128 (insn_ptr, &reg);
        reg = _DWARF_REG_TO_UNWIND_COLUMN (reg);
        if (_UNWIND_COLUMN_IN_RANGE (reg))
        fs->regs.reg[reg].how = REG_UNDEFINED;
        break;

    case DW_CFA_nop:
        break;

    case DW_CFA_register://The required action is to set the rule for the
    //first register to be register(R) where R is the second register.
        {
        _uleb128_t reg2;
        insn_ptr = read_uleb128 (insn_ptr, &reg);
        insn_ptr = read_uleb128 (insn_ptr, &reg2);
        reg = _DWARF_REG_TO_UNWIND_COLUMN (reg);
        if (_UNWIND_COLUMN_IN_RANGE (reg))
            {
            fs->regs.reg[reg].how = REG_SAVED_REG;
            fs->regs.reg[reg].loc.reg = (test_Unwind_Word)reg2;
            }
        }
        break;

    case DW_CFA_remember_state://The required action is to push the set of rules
    // for every register onto an implicit stack.
        {
        struct frame_state_reg_info *new_rs;
        if (unused_rs)
            {
        new_rs = unused_rs;
        unused_rs = unused_rs->prev;
            }
        else
            new_rs = alloca (sizeof (struct frame_state_reg_info));

        *new_rs = fs->regs;
        fs->regs.prev = new_rs;//saving always in the previous 
        }
        break;

    case DW_CFA_restore_state:
        {
        struct frame_state_reg_info *old_rs = fs->regs.prev;
        fs->regs = *old_rs;
        old_rs->prev = unused_rs;
        unused_rs = old_rs;
        }
        break;

    case DW_CFA_def_cfa://The required action is to define the current 
    //CFA rule to use the provided register and offset.
        insn_ptr = read_uleb128 (insn_ptr, &utmp);
        fs->regs.cfa_reg = (test_Unwind_Word)utmp;
        insn_ptr = read_uleb128 (insn_ptr, &utmp);
        fs->regs.cfa_offset = (test_Unwind_Word)utmp;
        fs->regs.cfa_how = CFA_REG_OFFSET;
        break;

    case DW_CFA_def_cfa_register:////The required action is to define the
    //current CFA rule to use the provided register (but to keep the old offset). This
    //operation is valid only if the current CFA rule is defined to use a register and offset.
        insn_ptr = read_uleb128 (insn_ptr, &utmp);
        fs->regs.cfa_reg = (test_Unwind_Word)utmp;
        fs->regs.cfa_how = CFA_REG_OFFSET;
        break;

    case DW_CFA_def_cfa_offset:
        insn_ptr = read_uleb128 (insn_ptr, &utmp);
        fs->regs.cfa_offset = utmp;
        /* cfa_how deliberately not set.  *///<<>> why
        break;

    case DW_CFA_def_cfa_expression:
        fs->regs.cfa_exp = insn_ptr;
        fs->regs.cfa_how = CFA_EXP;
        insn_ptr = read_uleb128 (insn_ptr, &utmp);
        insn_ptr += utmp;
        break;

    case DW_CFA_expression:
        insn_ptr = read_uleb128 (insn_ptr, &reg);
        reg = _DWARF_REG_TO_UNWIND_COLUMN (reg);
        if (_UNWIND_COLUMN_IN_RANGE (reg))
        {
            fs->regs.reg[reg].how = REG_SAVED_EXP;
            fs->regs.reg[reg].loc.exp = insn_ptr;
        }
        insn_ptr = read_uleb128 (insn_ptr, &utmp);
        insn_ptr += utmp;
        break;

        /* Dwarf3.  */
    case DW_CFA_offset_extended_sf://This instruction is identical to DW_CFA_offset_extended
        //except that the second operand is signed and factored. The resulting offset is
        //factored_offset * data_alignment_factor.
        insn_ptr = read_uleb128 (insn_ptr, &reg);
        insn_ptr = read_sleb128 (insn_ptr, &stmp);
        offset = stmp * fs->data_align;
        reg = _DWARF_REG_TO_UNWIND_COLUMN (reg);
        if (_UNWIND_COLUMN_IN_RANGE (reg))
        {
            fs->regs.reg[reg].how = REG_SAVED_OFFSET;
            fs->regs.reg[reg].loc.offset = offset;
        }
        break;

    case DW_CFA_def_cfa_sf:
        insn_ptr = read_uleb128 (insn_ptr, &utmp);
        fs->regs.cfa_reg = (test_Unwind_Word)utmp;
        insn_ptr = read_sleb128 (insn_ptr, &stmp);
        fs->regs.cfa_offset = (test_Unwind_Sword)stmp;
        fs->regs.cfa_how = CFA_REG_OFFSET;
        fs->regs.cfa_offset *= fs->data_align;
        break;

    case DW_CFA_def_cfa_offset_sf:
        insn_ptr = read_sleb128 (insn_ptr, &stmp);
        fs->regs.cfa_offset = (test_Unwind_Sword)stmp;
        fs->regs.cfa_offset *= fs->data_align;
        /* cfa_how deliberately not set.  */
        break;

    case DW_CFA_val_offset:
        insn_ptr = read_uleb128 (insn_ptr, &reg);
        insn_ptr = read_uleb128 (insn_ptr, &utmp);
        offset = (test_Unwind_Sword) utmp * fs->data_align;
        reg = _DWARF_REG_TO_UNWIND_COLUMN (reg);
        if (_UNWIND_COLUMN_IN_RANGE (reg))
        {
            fs->regs.reg[reg].how = REG_SAVED_VAL_OFFSET;
            fs->regs.reg[reg].loc.offset = offset;
        }
        break;

    case DW_CFA_val_offset_sf:
        insn_ptr = read_uleb128 (insn_ptr, &reg);
        insn_ptr = read_sleb128 (insn_ptr, &stmp);
        offset = stmp * fs->data_align;
        reg = _DWARF_REG_TO_UNWIND_COLUMN (reg);
        if (_UNWIND_COLUMN_IN_RANGE (reg))
        {
            fs->regs.reg[reg].how = REG_SAVED_VAL_OFFSET;
            fs->regs.reg[reg].loc.offset = offset;
        }
        break;

    case DW_CFA_val_expression:
        insn_ptr = read_uleb128 (insn_ptr, &reg);
        reg = _DWARF_REG_TO_UNWIND_COLUMN (reg);
        if (_UNWIND_COLUMN_IN_RANGE (reg))
        {
            fs->regs.reg[reg].how = REG_SAVED_VAL_EXP;
            fs->regs.reg[reg].loc.exp = insn_ptr;
        }
        insn_ptr = read_uleb128 (insn_ptr, &utmp);
        insn_ptr += utmp;
        break;

    case DW_CFA_GNU_window_save:
    #if defined (__aarch64__) && !defined (__ILP32__)
        /* This CFA is multiplexed with Sparc.  On AArch64 it's used to toggle
            return address signing status.  */
        fs->regs.reg[DWARF_REGNUM_AARCH64_RA_STATE].loc.offset ^= 1;
    #else
        /* ??? Hardcoded for SPARC register window configuration.  */
        if (_DWARF_FRAME_REGISTERS >= 32)
        for (reg = 16; reg < 32; ++reg)
            {
        fs->regs.reg[reg].how = REG_SAVED_OFFSET;
        fs->regs.reg[reg].loc.offset = (reg - 16) * sizeof (void *);
            }
    #endif
        break;

    case DW_CFA_GNU_args_size:
        insn_ptr = read_uleb128 (insn_ptr, &utmp);
        context->args_size = (test_Unwind_Word)utmp;
        break;

    case DW_CFA_GNU_negative_offset_extended:
        /* Obsoleted by DW_CFA_offset_extended_sf, but used by
            older PowerPC code.  */
        insn_ptr = read_uleb128 (insn_ptr, &reg);
        insn_ptr = read_uleb128 (insn_ptr, &utmp);
        offset = (test_Unwind_Word) utmp * fs->data_align;
        reg = _DWARF_REG_TO_UNWIND_COLUMN (reg);
        if (_UNWIND_COLUMN_IN_RANGE (reg))
        {
            fs->regs.reg[reg].how = REG_SAVED_OFFSET;
            fs->regs.reg[reg].loc.offset = -offset;
        }
        break;

    default:
    abort();
        //gcc_unreachable ();
    }
    }
}

test_Unwind_Word
test_execute_stack_op(const unsigned char *op_ptr, const unsigned char *op_end,
    struct test_Unwind_Context *context, test_Unwind_Word initial)
{
    test_Unwind_Word stack[64];	/* ??? Assume this is enough.  */
    int stack_elt;

    stack[0] = initial;
    stack_elt = 1;

    while (op_ptr < op_end)
    {
        enum dwarf_location_atom op = *op_ptr++;
        test_Unwind_Word result;
        _uleb128_t reg, utmp;
        _sleb128_t offset, stmp;

        switch (op)
    {
    case DW_OP_lit0:
    case DW_OP_lit1:
    case DW_OP_lit2:
    case DW_OP_lit3:
    case DW_OP_lit4:
    case DW_OP_lit5:
    case DW_OP_lit6:
    case DW_OP_lit7:
    case DW_OP_lit8:
    case DW_OP_lit9:
    case DW_OP_lit10:
    case DW_OP_lit11:
    case DW_OP_lit12:
    case DW_OP_lit13:
    case DW_OP_lit14:
    case DW_OP_lit15:
    case DW_OP_lit16:
    case DW_OP_lit17:
    case DW_OP_lit18:
    case DW_OP_lit19:
    case DW_OP_lit20:
    case DW_OP_lit21:
    case DW_OP_lit22:
    case DW_OP_lit23:
    case DW_OP_lit24:
    case DW_OP_lit25:
    case DW_OP_lit26:
    case DW_OP_lit27:
    case DW_OP_lit28:
    case DW_OP_lit29:
    case DW_OP_lit30:
    case DW_OP_lit31:
        result = op - DW_OP_lit0;
        break;

    case DW_OP_addr:
        result = (test_Unwind_Word) (test_Unwind_Ptr) read_pointer (op_ptr);
        op_ptr += sizeof (void *);
        break;

    case DW_OP_GNU_encoded_addr:
        {
        test_Unwind_Ptr presult;
        op_ptr = read_encoded_value (context, *op_ptr, op_ptr+1, &presult);
        result = presult;
        }
        break;

    case DW_OP_const1u:
        result = read_1u (op_ptr);
        op_ptr += 1;
        break;
    case DW_OP_const1s:
        result = read_1s (op_ptr);
        op_ptr += 1;
        break;
    case DW_OP_const2u:
        result = read_2u (op_ptr);
        op_ptr += 2;
        break;
    case DW_OP_const2s:
        result = read_2s (op_ptr);
        op_ptr += 2;
        break;
    case DW_OP_const4u:
        result = read_4u (op_ptr);
        op_ptr += 4;
        break;
    case DW_OP_const4s:
        result = read_4s (op_ptr);
        op_ptr += 4;
        break;
    case DW_OP_const8u:
        result = read_8u (op_ptr);
        op_ptr += 8;
        break;
    case DW_OP_const8s:
        result = read_8s (op_ptr);
        op_ptr += 8;
        break;
    case DW_OP_constu:
        op_ptr = read_uleb128 (op_ptr, &utmp);
        result = (test_Unwind_Word)utmp;
        break;
    case DW_OP_consts:
        op_ptr = read_sleb128 (op_ptr, &stmp);
        result = (test_Unwind_Sword)stmp;
        break;

    case DW_OP_reg0:
    case DW_OP_reg1:
    case DW_OP_reg2:
    case DW_OP_reg3:
    case DW_OP_reg4:
    case DW_OP_reg5:
    case DW_OP_reg6:
    case DW_OP_reg7:
    case DW_OP_reg8:
    case DW_OP_reg9:
    case DW_OP_reg10:
    case DW_OP_reg11:
    case DW_OP_reg12:
    case DW_OP_reg13:
    case DW_OP_reg14:
    case DW_OP_reg15:
    case DW_OP_reg16:
    case DW_OP_reg17:
    case DW_OP_reg18:
    case DW_OP_reg19:
    case DW_OP_reg20:
    case DW_OP_reg21:
    case DW_OP_reg22:
    case DW_OP_reg23:
    case DW_OP_reg24:
    case DW_OP_reg25:
    case DW_OP_reg26:
    case DW_OP_reg27:
    case DW_OP_reg28:
    case DW_OP_reg29:
    case DW_OP_reg30:
    case DW_OP_reg31:
        result = test_Unwind_GetGR (context, op - DW_OP_reg0);
        break;
    case DW_OP_regx:
        op_ptr = read_uleb128 (op_ptr, &reg);
        result = test_Unwind_GetGR (context, reg);
        break;

    case DW_OP_breg0:
    case DW_OP_breg1:
    case DW_OP_breg2:
    case DW_OP_breg3:
    case DW_OP_breg4:
    case DW_OP_breg5:
    case DW_OP_breg6:
    case DW_OP_breg7:
    case DW_OP_breg8:
    case DW_OP_breg9:
    case DW_OP_breg10:
    case DW_OP_breg11:
    case DW_OP_breg12:
    case DW_OP_breg13:
    case DW_OP_breg14:
    case DW_OP_breg15:
    case DW_OP_breg16:
    case DW_OP_breg17:
    case DW_OP_breg18:
    case DW_OP_breg19:
    case DW_OP_breg20:
    case DW_OP_breg21:
    case DW_OP_breg22:
    case DW_OP_breg23:
    case DW_OP_breg24:
    case DW_OP_breg25:
    case DW_OP_breg26:
    case DW_OP_breg27:
    case DW_OP_breg28:
    case DW_OP_breg29:
    case DW_OP_breg30:
    case DW_OP_breg31:
        op_ptr = read_sleb128 (op_ptr, &offset);
        result = test_Unwind_GetGR (context, op - DW_OP_breg0) + offset;
        break;
    case DW_OP_bregx:
        op_ptr = read_uleb128 (op_ptr, &reg);
        op_ptr = read_sleb128 (op_ptr, &offset);
        result = test_Unwind_GetGR (context, reg) + (test_Unwind_Word)offset;
        break;

    case DW_OP_dup:
        //gcc_assert (stack_elt);
        result = stack[stack_elt - 1];
        break;

    case DW_OP_drop:
        //gcc_assert (stack_elt);
        stack_elt -= 1;
        goto no_push;

    case DW_OP_pick:
        offset = *op_ptr++;
        //gcc_assert (offset < stack_elt - 1);
        result = stack[stack_elt - 1 - offset];
        break;

    case DW_OP_over:
        //gcc_assert (stack_elt >= 2);
        result = stack[stack_elt - 2];
        break;

    case DW_OP_swap:
        {
        test_Unwind_Word t;
        //gcc_assert (stack_elt >= 2);
        t = stack[stack_elt - 1];
        stack[stack_elt - 1] = stack[stack_elt - 2];
        stack[stack_elt - 2] = t;
        goto no_push;
        }

    case DW_OP_rot:
        {
        test_Unwind_Word t1, t2, t3;

        //gcc_assert (stack_elt >= 3);
        t1 = stack[stack_elt - 1];
        t2 = stack[stack_elt - 2];
        t3 = stack[stack_elt - 3];
        stack[stack_elt - 1] = t2;
        stack[stack_elt - 2] = t3;
        stack[stack_elt - 3] = t1;
        goto no_push;
        }

    case DW_OP_deref:
    case DW_OP_deref_size:
    case DW_OP_abs:
    case DW_OP_neg:
    case DW_OP_not:
    case DW_OP_plus_uconst:
        /* Unary operations.  */
        //gcc_assert (stack_elt);
        stack_elt -= 1;

        result = stack[stack_elt];

        switch (op)
        {
        case DW_OP_deref:
            {
        void *ptr = (void *) (test_Unwind_Ptr) result;
        result = (test_Unwind_Ptr) read_pointer (ptr);
            }
            break;

        case DW_OP_deref_size:
            {
        void *ptr = (void *) (test_Unwind_Ptr) result;
        switch (*op_ptr++)
            {
            case 1:
            result = read_1u (ptr);
            break;
            case 2:
            result = read_2u (ptr);
            break;
            case 4:
            result = read_4u (ptr);
            break;
            case 8:
            result = read_8u (ptr);
            break;
            default:
            abort();
            //gcc_unreachable ();
            }
            }
            break;

        case DW_OP_abs:
            if ((test_Unwind_Sword) result < 0)
        result = -result;
            break;
        case DW_OP_neg:
            result = -result;
            break;
        case DW_OP_not:
            result = ~result;
            break;
        case DW_OP_plus_uconst:
            op_ptr = read_uleb128 (op_ptr, &utmp);
            result += (test_Unwind_Word)utmp;
            break;

        default:
            abort();
            //gcc_unreachable ();
        }
        break;

    case DW_OP_and:
    case DW_OP_div:
    case DW_OP_minus:
    case DW_OP_mod:
    case DW_OP_mul:
    case DW_OP_or:
    case DW_OP_plus:
    case DW_OP_shl:
    case DW_OP_shr:
    case DW_OP_shra:
    case DW_OP_xor:
    case DW_OP_le:
    case DW_OP_ge:
    case DW_OP_eq:
    case DW_OP_lt:
    case DW_OP_gt:
    case DW_OP_ne:
        {
        /* Binary operations.  */
        test_Unwind_Word first, second;
        //gcc_assert (stack_elt >= 2);
        stack_elt -= 2;

        second = stack[stack_elt];
        first = stack[stack_elt + 1];

        switch (op)
            {
            case DW_OP_and:
        result = second & first;
        break;
            case DW_OP_div:
        result = (test_Unwind_Sword) second / (test_Unwind_Sword) first;
        break;
            case DW_OP_minus:
        result = second - first;
        break;
            case DW_OP_mod:
        result = second % first;
        break;
            case DW_OP_mul:
        result = second * first;
        break;
            case DW_OP_or:
        result = second | first;
        break;
            case DW_OP_plus:
        result = second + first;
        break;
            case DW_OP_shl:
        result = second << first;
        break;
            case DW_OP_shr:
        result = second >> first;
        break;
            case DW_OP_shra:
        result = (test_Unwind_Sword) second >> first;
        break;
            case DW_OP_xor:
        result = second ^ first;
        break;
            case DW_OP_le:
        result = (test_Unwind_Sword) second <= (test_Unwind_Sword) first;
        break;
            case DW_OP_ge:
        result = (test_Unwind_Sword) second >= (test_Unwind_Sword) first;
        break;
            case DW_OP_eq:
        result = (test_Unwind_Sword) second == (test_Unwind_Sword) first;
        break;
            case DW_OP_lt:
        result = (test_Unwind_Sword) second < (test_Unwind_Sword) first;
        break;
            case DW_OP_gt:
        result = (test_Unwind_Sword) second > (test_Unwind_Sword) first;
        break;
            case DW_OP_ne:
        result = (test_Unwind_Sword) second != (test_Unwind_Sword) first;
        break;

            default:
            abort();
        //gcc_unreachable ();
            }
        }
        break;

    case DW_OP_skip:
        offset = read_2s (op_ptr);
        op_ptr += 2;
        op_ptr += offset;
        goto no_push;

    case DW_OP_bra:
        //gcc_assert (stack_elt);
        stack_elt -= 1;

        offset = read_2s (op_ptr);
        op_ptr += 2;
        if (stack[stack_elt] != 0)
        op_ptr += offset;
        goto no_push;

    case DW_OP_nop:
        goto no_push;

    default:
        abort();
        //gcc_unreachable ();
    }

        /* Most things push a result value.  */
        //gcc_assert ((size_t) stack_elt < sizeof(stack)/sizeof(*stack));
        stack[stack_elt++] = result;
    no_push:;
    }

    /* We were executing this program to get a value.  It should be
        at top of stack.  */
    //gcc_assert (stack_elt);
    stack_elt -= 1;
    return stack[stack_elt];
}

/*  Context management  */
void
_update_context(struct test_Unwind_Context *context, test_Unwind_FrameState *fs)
{
    struct test_Unwind_Context orig_context = *context;
    void *cfa;

    /*  Special Handling: check gcc equivelant  */

    /*  Compute the CFA  */
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

    /*  Compute all registers  */
    for (int i = 0; i < _DWARF_FRAME_REGISTERS + 1; ++i)
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
    /*  checking for sigreturn()  */
    if ((pc[0] == 0x38007777 || pc[0] == 0x38000077
        || pc[0] == 0x38006666 || pc[0] == 0x380000AC)
        && pc[1] == 0x44000002)
        test_Unwind_SetSignalFrame (context, 1);
    #endif
}

void __attribute__((noinline))
_init_context(struct test_Unwind_Context **context_ptr, void *outer_cfa, void *outer_ra)
{
    void *ra = __builtin_extract_return_addr(__builtin_return_address(0));
    test_Unwind_FrameState *fs;
    test_Unwind_SpTmp sp_slot;
    struct test_Unwind_Context *context;

    context = (struct test_Unwind_Context *)malloc(sizeof(struct test_Unwind_Context));
    memset(context, 0, sizeof(struct test_Unwind_Context));
    context->ra = ra;
    if (!ASSUME_EXTENDED_UNWIND_CONTEXT)
        context->flags |= EXTENDED_CONTEXT_BIT;

    test_uw_frame_state_for(context, &fs);

    if (dwarf_reg_size_table[0] == 0)
        _builtin_init_dwarf_reg_size_table(dwarf_reg_size_table);

    /*  Force the frame state to use the known cfa value.  */ //TODO: we should check why??
    test_Unwind_SetSpColumn(context, outer_cfa, &sp_slot);
    fs->regs.cfa_how = CFA_REG_OFFSET;
    fs->regs.cfa_reg = _builtin_dwarf_sp_column();
    fs->regs.cfa_offset = 0;

    _update_context(context, fs);

    context->ra = __builtin_extract_return_addr(outer_ra);
    /*  Special Handling: aarch64 needs modification for return address value  */
    *context_ptr = context;
}

void
test_uw_update_context(struct test_Unwind_Context *context, test_Unwind_FrameState *fs)
{
    _update_context(context, fs);

    if (fs->regs.reg[_DWARF_REG_TO_UNWIND_COLUMN(fs->retaddr_column)].how == REG_UNDEFINED)
        context->ra = 0;    /*  outermost stack frame  */
    else
        context->ra = __builtin_extract_return_addr(test_Unwind_GetPtr(context, fs->retaddr_column));
    /*  Special Handling: aarch64 needs modification for return address value  */
}

test_Unwind_Ptr
test_uw_identify_context(struct test_Unwind_Context *context)
{
    // TODO: This needs more resreach
    if (__STACK_GROWS_DOWNWARD__)
        return test_Unwind_GetCFA(context) - test_Unwind_IsSignalFrame(context);
    else
        return test_Unwind_GetCFA(context) + test_Unwind_IsSignalFrame(context);
}

void
uw_copy_context(struct test_Unwind_Context **target, struct test_Unwind_Context *source)
{
    struct test_Unwind_Context * temp;
    temp = (struct test_Unwind_Context *)malloc(sizeof(struct test_Unwind_Context));
    *temp = *source;
    *target = temp;
}

long
_install_context(struct test_Unwind_Context *current, struct test_Unwind_Context *target)
{
    // test_Unwind_SpTmp sp_slot;

    /*  If the target frame does not have a saved stack pointer,
        then set up the target's CFA.  */
    // if (!test_Unwind_GetGRPtr(target, _builtin_dwarf_sp_column()))
    //     test_Unwind_SetSpColumn(target, target->cfa, &sp_slot);

    for (int i = 0; i < _DWARF_FRAME_REGISTERS; ++i) {
        void *c = (void *)(test_Unwind_Internal_Ptr)current->reg[i];
        void *t = (void *)(test_Unwind_Internal_Ptr)target->reg[i];
            
        if (target->by_value[i] && c) {
            test_Unwind_Word w;
            test_Unwind_Ptr p;
            if (dwarf_reg_size_table[i] == sizeof(test_Unwind_Word)) {
                w = (test_Unwind_Internal_Ptr)t;
                memcpy(c, &w, sizeof (test_Unwind_Word));
            }
            else {
                p = (test_Unwind_Internal_Ptr)t;
                memcpy(c, &p, sizeof (test_Unwind_Ptr));
            }
        } else if (t && c && t != c) {
            memcpy(c, t, dwarf_reg_size_table[i]);
        }
    }

    if (__STACK_GROWS_DOWNWARD__)
        return target->cfa - current->cfa + target->args_size;
    else
        return current->cfa - target->cfa - target->args_size;

    /*  If the current frame doesn't have a saved stack pointer, then we
        need to rely on EH_RETURN_STACKADJ_RTX to get our target stack
        pointer value reloaded.  */
    /*  This was the original code from libgcc. You might be wondering why
        would libgcc do it this way. The answer is, we don't know, and we
        don't really care at this point. If you really want to know then you
        might want to check out how __builtin_eh_return() works first, then,
        and only then, come back here.  */
    // if (!test_Unwind_GetGRPtr (current, _builtin_dwarf_sp_column())) {
    //     void *target_cfa;
    //     target_cfa = test_Unwind_GetPtr (target, _builtin_dwarf_sp_column());

    //     /*  We adjust SP by the difference between CURRENT and TARGET's CFA.  */
    //     if (__STACK_GROWS_DOWNWARD__)
    //         return target_cfa - current->cfa + target->args_size;
    //     else
    //         return current->cfa - target_cfa - target->args_size;
    // }
    // return 0;
}

void *
test_uw_frob_return_addr(struct test_Unwind_Context *current __attribute__((__unused__)),
    struct test_Unwind_Context *target)
{
    void *ret_addr = __builtin_frob_return_addr(target->ra); //TODO: Check if this is avilable to use directly: https://gcc.gnu.org/onlinedocs/gcc/Return-Address.html
    return ret_addr;
} 

void
uw_free_context(struct test_Unwind_Context * context)
{
    free(context);
}

/*  Frame State management  */
const unsigned char *
test_extract_cie_info(const struct test_dwarf_cie *cie, struct test_Unwind_Context *context,
    test_Unwind_FrameState *fs)
{
    const unsigned char *aug = cie->augmentation;
    const unsigned char *p = aug + strlen((const char *)aug) + 1;
    const unsigned char *ret = NULL;
    _uleb128_t utmp;
    _sleb128_t stmp;

    /*  g++ v2 "eh" has pointer immediately following augmentation string,
        so it must be handled first.  */
    if (aug[0] == 'e' && aug[1] == 'h') {
        //fs->eh_ptr = read_pointer (p); //TODO(DONE): Check for read_pointer(). It should work now, hopfully
        fs->eh_ptr = (void *)(*(test_Unwind_Internal_Ptr *)p);
        p += sizeof(test_Unwind_Internal_Ptr *);
        aug += 2;
    }

    /*  After the augmentation resp. pointer for "eh" augmentation
        follows for CIE version >= 4 address size byte and
        segment size byte.  */
    if (cie->version >= 4) {
        if (p[0] != sizeof(void *) || p[1] != 0)
        return NULL;
        p += 2;
    }

    /*  Immediately following this are the code and
        data alignment and return address column.  */
    p = read_uleb128(p, &utmp);
    fs->code_align = (test_Unwind_Word)utmp;
    p = read_sleb128(p, &stmp);
    fs->data_align = (test_Unwind_Sword)stmp;
    if (cie->version == 1) {
        fs->retaddr_column = *p++;
    } else {
        p = read_uleb128(p, &utmp);
        fs->retaddr_column = (test_Unwind_Word)utmp;
    }
    fs->lsda_encoding = DW_EH_PE_omit;

    /*  If the augmentation starts with 'z', then a uleb128 immediately
        follows containing the length of the augmentation field following
        the size.  */
    if (*aug == 'z') {
        p = read_uleb128(p, &utmp);
        ret = p + utmp;
        fs->saw_z = 1;
        ++aug;
    }

    /*  Iterate over recognized augmentation subsequences.  */
    while (*aug != '\0') {
        if (aug[0] == 'L') {
            /*  "L" indicates a byte showing how the LSDA pointer is encoded.  */
            fs->lsda_encoding = *p++;
            aug += 1;
        } else if (aug[0] == 'R') {
            /*  "R" indicates a byte indicating how FDE addresses are encoded.  */
            fs->fde_encoding = *p++;
            aug += 1;
        } else if (aug[0] == 'P') {
            /*  "P" indicates a personality routine in the CIE augmentation.  */
            test_Unwind_Ptr personality;
            p = read_encoded_value(context, *p, p + 1, &personality);
            fs->personality = (test_Unwind_Personality_Fn) personality;
            aug += 1;
        } else if (aug[0] == 'S') {
            /*  "S" indicates a signal frame.  */
            fs->signal_frame = 1;
            aug += 1;
        } else if (aug[0] == 'B') {
            /*  aarch64 B-key pointer authentication.  */
            aug += 1;
        } else {
            /*  Otherwise we have an unknown augmentation string.
            Bail unless we saw a 'z' prefix.  */
            return ret;
        }
    }

    return ret ? ret : p;
}

test_Unwind_Reason_Code
test_uw_frame_state_for(struct test_Unwind_Context *context, test_Unwind_FrameState **fs_ptr)
{
    const struct test_dwarf_fde *fde;
    const struct test_dwarf_cie *cie;
    const unsigned char *aug, *insn, *end;
    test_Unwind_FrameState *fs;

    fs = (test_Unwind_FrameState *)malloc(sizeof(test_Unwind_FrameState));
    memset(fs, 0, sizeof(test_Unwind_FrameState));
    context->args_size = 0;
    context->lsda = 0;

    if (context->ra == 0)
        return _URC_END_OF_STACK;
    
    fde = find_fde(context->ra + test_Unwind_IsSignalFrame(context) - 1, &context->bases);
    if (fde == NULL)
        return _URC_END_OF_STACK;

    fs->pc = context->bases.func;
    cie = test_get_cie(fde);
    insn = test_extract_cie_info(cie, context, fs);
    /*  CIE contained unknown augmentation.  */
    if (insn == NULL)
        return _URC_FATAL_PHASE1_ERROR;

    /*  First decode all the insns in the CIE.  */
    end = (const unsigned char *)test_next_fde((const struct test_dwarf_fde *)cie);
    test_execute_cfa_program(insn, end, context, fs);

    /*  Locate augmentation for the fde.  */
    aug = (const unsigned char *)fde + sizeof(*fde);
    aug += 2 * size_of_encoded_value(fs->fde_encoding);
    insn = NULL;

    if (fs->saw_z) {
        _uleb128_t i;
        aug = read_uleb128(aug, &i);
        insn = aug + i;
    }

    if (fs->lsda_encoding != DW_EH_PE_omit) {
        test_Unwind_Ptr lsda;
        aug = read_encoded_value(context, fs->lsda_encoding, aug, &lsda);
        context->lsda = (void *)lsda;
        //printf("generated lsda: 0x%lx\n", lsda);
    }

    /*  Then the insns in the FDE up to our target PC.  */
    if (insn == NULL)
        insn = aug;
    end = (const unsigned char *)test_next_fde(fde);
    test_execute_cfa_program(insn, end, context, fs);

    *fs_ptr = fs;
    return _URC_NO_REASON;
}

test_Unwind_Personality_Fn
uw_get_personality(test_Unwind_FrameState *fs)
{
    return fs->personality;
}

#ifdef __cplusplus
}
#endif