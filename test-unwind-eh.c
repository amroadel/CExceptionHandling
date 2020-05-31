#include "test-unwind-eh.h"
#include "test-unwind-pe.h"
#include "test-unwind.c"

#ifdef __cplusplus
extern "C" {
#endif

/* Routines */
static test_Unwind_Ptr
find_fde(const unsigned char *eh_frame_hdr);

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
    test_Unwind_Ptr pc_begin;
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
    pc_begin = *(fde + fde_id_offset + 4); //The starting address to which this FDE applies. 

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
    unsigned char *cie_aug_p = cie_aug; //auxillary pointer to the start of the cie_aug section 
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

      //fde_aug = read_encoded_value (context, lsda_encoding, fde_aug, &lsda); //NOTE : This funtion is incomplete 
      fde_aug = read_encoded_value_with_base(lsda_encoding, pc_begin, fde_aug, &lsda);
      context->lsda = (void *) lsda;
    }

}
#ifdef __cplusplus
}
#endif