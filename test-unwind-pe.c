#include "test-unwind-pe.h"
#include "stdlib.h"
#include "stdio.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
size_of_encoded_value (unsigned char encoding)
{
    if (encoding == DW_EH_PE_omit)
        return 0;

    switch (encoding & 0x07) {
        case DW_EH_PE_absptr:
            return sizeof(void *);
        case DW_EH_PE_udata2:
            return 2;
        case DW_EH_PE_udata4:
            return 4;
        case DW_EH_PE_udata8:
            return 8;
        case DW_EH_PE_sdata2:
            return 2;
        case DW_EH_PE_sdata4:
            return 4;
        case DW_EH_PE_sdata8:
            return 8;
    }
    abort();
}

const unsigned char *
read_uleb128 (const unsigned char *p, _uleb128_t *val)
{
    unsigned int shift = 0;
    unsigned char byte;
    _uleb128_t result;

    result = 0;
    do {
        byte = *p++;
        result |= ((_uleb128_t)byte & 0x7f) << shift;
        shift += 7;
    } while (byte & 0x80);

    *val = result;
    return p;
}

const unsigned char *
read_sleb128 (const unsigned char *p, _sleb128_t *val)
{
    unsigned int shift = 0;
    unsigned char byte;
    _uleb128_t result;

    result = 0;
    do {
        byte = *p++;
        result |= ((_uleb128_t)byte & 0x7f) << shift;
        shift += 7;
    } while (byte & 0x80);

    if (shift < 8 * sizeof(result) && (byte & 0x40) != 0)
    result |= -(((_uleb128_t)1L) << shift);

    *val = (_sleb128_t) result;
    return p;
}

const unsigned char *
read_encoded_value_with_base (unsigned char encoding, test_Unwind_Ptr base,
			      const unsigned char *p, test_Unwind_Ptr *val)
{
    union unaligned {
        void *ptr;
        unsigned u2 __attribute__ ((mode (HI)));
        unsigned u4 __attribute__ ((mode (SI)));
        unsigned u8 __attribute__ ((mode (DI)));
        signed s2 __attribute__ ((mode (HI)));
        signed s4 __attribute__ ((mode (SI)));
        signed s8 __attribute__ ((mode (DI)));
    } __attribute__((__packed__));

    const union unaligned *u = (const union unaligned *) p;
    test_Unwind_Ptr result;

    if (encoding == DW_EH_PE_aligned) {
        test_Unwind_Ptr a = (test_Unwind_Ptr) p;
        a = (a + sizeof (void *) - 1) & - sizeof(void *);
        result = *(test_Unwind_Ptr *) a;
        p = (const unsigned char *) (test_Unwind_Ptr) (a + sizeof (void *));
    } else {
        switch (encoding & 0x0f) {
            case DW_EH_PE_absptr:
                result = (test_Unwind_Ptr) u->ptr;
                printf("result1 is %p\n", result);
                p += sizeof (void *);
                break;

            case DW_EH_PE_uleb128: {
                _uleb128_t tmp;
                p = read_uleb128 (p, &tmp);
                result = (test_Unwind_Ptr) tmp;
                } break;

            case DW_EH_PE_sleb128: {
                _sleb128_t tmp;
                p = read_sleb128 (p, &tmp);
                result = (test_Unwind_Ptr) tmp;
                } break;

            case DW_EH_PE_udata2:
                result = u->u2;
                p += 2;
                break;
            case DW_EH_PE_udata4:
                result = u->u4;
                p += 4;
                break;
            case DW_EH_PE_udata8:
                result = u->u8;
                p += 8;
                break;

            case DW_EH_PE_sdata2:
                result = u->s2;
                p += 2;
                break;
            case DW_EH_PE_sdata4:
                result = u->s4;
                p += 4;
                break;
            case DW_EH_PE_sdata8:
                result = u->s8;
                p += 8;
                break;

            default:
                abort();
        }

        switch (encoding & 0x70) {
            case DW_EH_PE_pcrel:
                result += (test_Unwind_Ptr) u;
                break;
            case DW_EH_PE_textrel:
            case DW_EH_PE_datarel:
            case DW_EH_PE_funcrel:
            printf("baseX is %p\n", base);
                result += base;
            printf("result is %p\n", result);
                break;
            default:
                break;
        }
        if (encoding & DW_EH_PE_indirect) {
            if (result != 0)
                result = *(test_Unwind_Ptr *) result;
            else
                abort();
        }
    }

    *val = result;
    return p;
}

/*
#ifndef NO_BASE_OF_ENCODED_VALUE

/* Given an encoding and an _Unwind_Context, return the base to which
   the encoding is relative.  This base may then be passed to
   read_encoded_value_with_base for use when the _Unwind_Context is
   not available.  

//TODO: define the used unwind functions 
test_Unwind_Ptr
base_of_encoded_value (unsigned char encoding, struct test_Unwind_Context *context)
{
  if (encoding == DW_EH_PE_omit)
    return 0;

  switch (encoding & 0x70)
    {
    case DW_EH_PE_absptr:
    case DW_EH_PE_pcrel:
    case DW_EH_PE_aligned:
      return 0;

    case DW_EH_PE_textrel:
      return _Unwind_GetTextRelBase (context);
    case DW_EH_PE_datarel:
      return _Unwind_GetDataRelBase (context);
    case DW_EH_PE_funcrel:
      return _Unwind_GetRegionStart (context);
    }
  abort();
}

#endif

/* Like read_encoded_value_with_base, but get the base from the context
   rather than providing it directly.  *

static inline const unsigned char *
read_encoded_value (struct test_Unwind_Context *context, unsigned char encoding,
		    const unsigned char *p, test_Unwind_Ptr *val)
{
  return read_encoded_value_with_base (encoding,
		base_of_encoded_value (encoding, context),
		p, val);
}*/

#ifdef __cplusplus
}
#endif