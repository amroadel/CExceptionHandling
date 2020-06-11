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

inline const struct test_dwarf_cie *
test_get_cie(const struct test_dwarf_fde *f)
{
    return (const void *)&f->CIE_delta - f->CIE_delta;
}

test_Unwind_Ptr
base_from_header (unsigned char encoding)
{
    if (encoding == DW_EH_PE_omit)
    return 0;

    switch (encoding & 0x70) {
    case DW_EH_PE_absptr:
    case DW_EH_PE_pcrel:
    case DW_EH_PE_aligned:
        return 0;

    case DW_EH_PE_textrel:
        return (test_Unwind_Ptr) header.eh_bases.tbase;
    case DW_EH_PE_datarel:
        return (test_Unwind_Ptr) header.eh_bases.dbase;
    default:
        abort();
    }
}

const test_fde *
linear_search_fde(void *pc)
{
    const unsigned char *p = header.entries;
    test_Unwind_Ptr ip = (test_Unwind_Ptr)pc;
    test_Unwind_Ptr base;
    test_Unwind_Ptr fde;

    p = read_encoded_value_with_base(header.entry_encoding,
    (test_Unwind_Ptr)header.self, p, &base);
    
    for (int i = 0; i < header.count; i++) {
        p = read_encoded_value_with_base(header.entry_encoding,
        (test_Unwind_Ptr)header.self, p, &fde);

        if (ip > base) {
            if (i + 1 == header.count)
                return (const test_fde *)fde;
            p = read_encoded_value_with_base(header.entry_encoding,
            (test_Unwind_Ptr)header.self, p, &base);
            if (base > ip)
                return (const test_fde *)fde;
        } else {
            return NULL;
        }            
    }
    abort();
}

const test_fde *
find_fde(void *pc, struct test_dwarf_eh_bases *bases) // TODO: make the search binary in a separate function
{
    if (header.entries == NULL || header.count == 0)
        abort();
    const test_fde *fde = linear_search_fde(pc);
    
    if (fde) {
        bases->tbase = header.eh_bases.tbase;
        bases->dbase = header.eh_bases.dbase;

        unsigned char encoding;
        test_Unwind_Ptr func;
        encoding = test_get_fde_encoding (fde);
        read_encoded_value_with_base (encoding, base_from_header(encoding),
            fde->pc_begin, &func);
        bases->func = (void *) func;
    }
}

#ifdef __cplusplus
}
#endif