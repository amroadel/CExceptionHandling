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
struct test_Unwind_Context {
    void *cfa;
    void *ra;
    void *lsda;
    struct eh_bases bases;
    // keep them for now until we know more about them
    test_Unwind_Word flags;
    test_Unwind_Word version;
    test_Unwind_Word args_size;
};

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
static void
fill_context(const unsigned char * fde, struct test_Unwind_Context *context)
{
     add_lsda(fde, context);
     
}
static void
add_lsda(const unsigned char *fde, struct test_Unwind_Context *context)
{
    unsigned char lsda_encoding;
    unsigned char fde_encoding;
    void *lsda;
    const unsigned char *cie; 
    int fde_legnth  = *fde;
    int cie_legnth; 
    int cie_offset_value;
    unsigned char cie_version;
    const unsigned char *cie_aug;
    const unsigned char *fde_aug;
    const unsigned char *p; 
    void *pc_begin;
    _uleb128_t utmp;
    _sleb128_t stmp;
    int cie_id_offset;
    int fde_id_offset; 
    char z_flag = 0;  

    if(fde_legnth == 0xffffffff)
        fde_id_offset =  12; //Legnth section + extended legnth section
    else 
        fde_id_offset = 4;

    cie_offset_value = *(fde + fde_id_offset); // the byte offset to the start of the CIE with which this FDE is associated   
    cie = (fde+fde_id_offset) - cie_offset_value; //the start of the cie (length record)
    pc_begin = (void *)(fde + fde_id_offset + 4); //The starting address to which this FDE applies. 
    context->bases.func = pc_begin;

    //TODO: if cie_legnth is 0, CIE shall be considered a terminator and the proccesing shall end
    cie_legnth = *cie;
    if(cie_legnth == 0xffffffff)
        cie_id_offset =  12; //Legnth section + extended legnth section
    else 
        cie_id_offset = 4;
    
    cie_version = *(cie + cie_id_offset + 4); // cie_version is either 1 or 3
    cie_aug = cie + cie_id_offset + 5;
    p = cie_aug; 
    p = read_uleb128 (p, &utmp);
    p = read_sleb128 (p, &stmp);

    if (cie_version == 1)
        p++;
    else
        p = read_uleb128 (p, &utmp);
    lsda_encoding = DW_EH_PE_omit;

    unsigned char aug_arr[6]; //hard codded for now (6 is the number of possible letters)
    const unsigned char *cie_aug_p = cie_aug; //auxillary pointer to the start of the cie_aug section 
    int i = 0;

    while (*cie_aug_p != '\0')
    {
      aug_arr[i] = *cie_aug_p;
      cie_aug_p++;
      i++;
    }
    aug_arr[i] = '\0';
    cie_aug = aug_arr;

    /* g++ v2 "eh" has pointer immediately following augmentation string,
       so it must be handled first.  */   
    if (cie_aug[0] == 'e' && cie_aug[1] == 'h')
    {
      p += sizeof (void *);
      cie_aug += 2;
    }
    
    /* If the augmentation starts with 'z', then a uleb128 immediately
     follows containing the length of the augmentation field following
     the size.  */
    if (*cie_aug == 'z')
    {
      p = read_uleb128 (p, &utmp);
      ++cie_aug;
      z_flag = 1;
    }

    while (*cie_aug != '\0')
    {

       /* "L" indicates a byte showing how the LSDA pointer is encoded.  */
      if (cie_aug[0] == 'L')
      {
        lsda_encoding = *p++;
        cie_aug += 1;      
      }
      /* "R" indicates a byte indicating how FDE addresses are encoded.  */
      else if (cie_aug[0] == 'R')
      {
        fde_encoding = *p++;
        cie_aug += 1;
      }
      else
      {
        cie_aug += 1;
      }
    } 
    
    fde_aug = fde + fde_id_offset + 4; //skip legnth and ID sections
    fde_aug += 2 * size_of_encoded_value (fde_encoding);
    if (z_flag)
    {
      _uleb128_t i;
      fde_aug = read_uleb128 (fde_aug, &i);
    }
  if (lsda_encoding != DW_EH_PE_omit)
    {
      test_Unwind_Ptr lsda;
       
      test_Unwind_Ptr base = (test_Unwind_Ptr) header.eh_frame;
      fde_aug = read_encoded_value_with_base(lsda_encoding, base, fde_aug, &lsda);
      
      //fde_aug = read_encoded_value (context, lsda_encoding, fde_aug, &lsda); //NOTE : This funtion is incomplete 
      context->lsda = (void *) lsda;
    }

}
#ifdef __cplusplus
}
#endif