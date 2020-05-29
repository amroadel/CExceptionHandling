#include "test-unwind-eh.h"
#include "test-unwind-pe.h"
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

/* Routines */
void
init_eh_frame_hdr(const unsigned char *eh_frame_hdr)
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
}

const unsigned char *
find_fde(void *ra)
{
    if (header.entries == NULL || header.count == 0)
        abort();
    const unsigned char *p = header.entries;

    test_Unwind_Ptr ip = (test_Unwind_Ptr)ra;
    test_Unwind_Ptr base;
    test_Unwind_Ptr fde;

    printf(" base before is %p\n", *(short *)p);
    p = read_encoded_value_with_base(header.entry_encoding,
    (test_Unwind_Ptr)header.self, p, &base);
    for (int i = 0; i < header.count; i++) {
        p = read_encoded_value_with_base(header.entry_encoding,
        (test_Unwind_Ptr)header.self, p, &fde);

        printf(" base is %p\n", base);

        if (ip > base) {
            if (i + 1 == header.count) {
                printf(" base is %p\n", base);
                printf(" ra is %p\n", ip);
                return (const unsigned char *)fde;
            }
            printf(" base before is %p\n", *(short *)p);
            p = read_encoded_value_with_base(header.entry_encoding,
            (test_Unwind_Ptr)header.self, p, &base);
            if (base > ip) {
                printf(" base is %p\n", base);
                printf(" ra is %p\n", ip);
                return (const unsigned char *)fde;
            }
        } else {
            if (i + 1 == header.count)
                abort();
            printf(" base before is %p\n", *(short *)p);
            p = read_encoded_value_with_base(header.entry_encoding,
            (test_Unwind_Ptr)header.self, p, &base);
        }            
    }
    abort();
}

// static void
// fill_context(const unsigned char *fde, test_Unwind_Context *context);

#ifdef __cplusplus
}
#endif