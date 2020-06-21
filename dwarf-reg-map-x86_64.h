#ifndef DWARF_REG_MAP
#define DWARF_REG_MAP

#ifdef __cplusplus
extern "C" {
#endif

#define _DWARF_FRAME_REGISTERS 17
#define _DWARF_REG_TO_UNWIND_COLUMN(n) n

#define _rax 0
#define _rdx 1
#define _rcx 2
#define _rbx 3
#define _rsi 4
#define _rdi 5
#define _rbp 6
#define _rsp 7
#define _r8  8
#define _r9  9
#define _r10 10
#define _r11 11
#define _r12 12
#define _r13 13
#define _r14 14
#define _r15 15
#define _ra  16

static inline int
_builtin_dwarf_sp_column()
{
    return (int)_rsp;
}

static inline void
_builtin_init_dwarf_reg_size_table(unsigned char *table)
{
    unsigned char *p = table;
    for (int i = 0; i < _DWARF_FRAME_REGISTERS; i++)
        *p++ = 8;
}

#ifdef __cplusplus
}
#endif

#endif /*  DWARF_REG_MAP  */