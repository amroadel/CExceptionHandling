#ifndef TEST_UNWIND_PE
#define TEST_UNWIND_PE

#include "test-unwind.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _UNWIND_H
typedef unsigned _uleb128_t __attribute__((__mode__(__pointer__)));
typedef unsigned _sleb128_t __attribute__((__mode__(__pointer__)));
typedef unsigned test_Unwind_Ptr __attribute__((__mode__(__pointer__)));
#endif

/*  Pointer encodings  */
/*  Special  */
#define DW_EH_PE_absptr         0x00
#define DW_EH_PE_omit           0xff
/*  Data encoding  */
#define DW_EH_PE_uleb128        0x01
#define DW_EH_PE_udata2         0x02
#define DW_EH_PE_udata4         0x03
#define DW_EH_PE_udata8         0x04
#define DW_EH_PE_signed         0x08
#define DW_EH_PE_sleb128        0x09
#define DW_EH_PE_sdata2         0x0A
#define DW_EH_PE_sdata4         0x0B
#define DW_EH_PE_sdata8         0x0C
/*  Base encoding  */
#define DW_EH_PE_pcrel          0x10
#define DW_EH_PE_textrel        0x20
#define DW_EH_PE_datarel        0x30
#define DW_EH_PE_funcrel        0x40
#define DW_EH_PE_aligned        0x50
#define DW_EH_PE_indirect	    0x80

/*  Routines  */
unsigned int
size_of_encoded_value(unsigned char encoding);

const unsigned char *
read_uleb128(const unsigned char *p, _uleb128_t *val);

const unsigned char *
read_sleb128(const unsigned char *p, _sleb128_t *val);

const unsigned char *
read_encoded_value_with_base(unsigned char encoding, test_Unwind_Ptr base,
	const unsigned char *p, test_Unwind_Ptr *val);

test_Unwind_Ptr
base_of_encoded_value(unsigned char encoding, struct test_Unwind_Context *context);

const unsigned char *
read_encoded_value(struct test_Unwind_Context *context, unsigned char encoding,
	const unsigned char *p, test_Unwind_Ptr *val);

#ifdef __cplusplus
}
#endif

#endif /*  TEST_UNWIND_PE  */