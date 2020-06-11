#include "test-unwind-fde.h"
#include "test-unwind-pe.h"
#include "stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

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

    header.eh_bases.tbase = (void *)text;
    header.eh_bases.dbase = (void *)eh_frame_hdr;
    header.eh_bases.func = NULL;
}

const unsigned char *
find_fde(void *ra, struct test_dwarf_eh_bases bases) // TODO: add the bases
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


inline const struct test_dwarf_cie *
test_get_cie (const struct test_dwarf_fde *f)
{
  return (const void *)&f->CIE_delta - f->CIE_delta;
}

#ifdef __cplusplus
}
#endif