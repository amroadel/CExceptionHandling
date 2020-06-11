#include "test-unwind-eh.h"
#include "test-unwind-pe.h"
#include "test-unwind-fde.h"
#include "dwarf-reg-map-x86_64.h"
#include "stdlib.h"
#include "stdio.h" // remember to delete this
#include <stddef.h> //for memset 

#ifdef __cplusplus
extern "C" {
#endif

/* Data types*/
struct test_Unwind_FrameState {
    /* Each register save state can be described in terms of a CFA slot,
        another register, or a location expression.  */
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

        /* The CFA can be described in terms of a reg+offset or a
            location expression.  */
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
};

struct test_Unwind_Context {
    void *cfa;
    void *ra;
    void *lsda;
    struct test_dwarf_eh_bases bases;
    // keep them for now until we know more about them
    test_Unwind_Word flags;
    test_Unwind_Word version;
    test_Unwind_Word args_size;
};

/*static void
fill_context(const unsigned char * fde, struct test_Unwind_Context *context)
{
     add_lsda(fde, context);
     
}*/
void
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
    //context->bases.func = pc_begin;

    //TODO: if cie_legnth is 0, CIE shall be considered a terminator and the proccesing shall end
    cie_legnth = *cie;
    printf("cie legnth: %i\n", cie_legnth);
    if(cie_legnth == 0xffffffff)
        cie_id_offset =  12; //Legnth section + extended legnth section
    else 
        cie_id_offset = 4;
    
    cie_version = *(cie + cie_id_offset + 4); // cie_version is either 1 or 3
    cie_aug = cie + cie_id_offset + 5;
    printf("cie_version %u\n", cie_version); 
    p = cie_aug + strlen ((const char *)cie_aug) + 1; // Skip the augmentation string.

    /* g++ v2 "eh" has pointer immediately following augmentation string,
       so it must be handled first.  */   
    if (cie_aug[0] == 'e' && cie_aug[1] == 'h')
    {
      p += sizeof (void *);
      cie_aug += 2;
    }

    p = read_uleb128 (p, &utmp);    /* Skip code alignment.  */
    p = read_sleb128 (p, &stmp);    /* Skip data alignment.  */


    if (cie_version == 1)
        p++;
    else
        p = read_uleb128 (p, &utmp);
    lsda_encoding = DW_EH_PE_omit;
    

   /* unsigned char aug_arr[6]; //hard codded for now (6 is the number of possible letters)
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
    int j = 0; 
    for (j; j <= i; j++)
        printf("value %i: %u\n", j, aug_arr[j]); */

    
    /* If the augmentation starts with 'z', then a uleb128 immediately
     follows containing the length of the augmentation field following
     the size.  */
    if (*cie_aug == 'z')
    {
      p = read_uleb128 (p, &utmp);  /* Skip augmentation length.  */
      ++cie_aug;
      z_flag = 1;
    }

    while (*cie_aug != '\0')
    {

       /* "L" indicates a byte showing how the LSDA pointer is encoded.  */
      if (cie_aug[0] == 'L')
      {
        printf("L\n");
        lsda_encoding = *p++;
        printf("lsda_encoding %u \n",lsda_encoding);
        cie_aug += 1;      
      }
      /* "R" indicates a byte indicating how FDE addresses are encoded.  */
      else if (cie_aug[0] == 'R')
      {
        printf("R\n");
        fde_encoding = *p++;
        cie_aug += 1;
      }
      else if (cie_aug[0] == 'P')
      {
        printf("P\n");
        test_Unwind_Ptr personality; 
        //p += sizeof (void *);
        //p = read_encoded_value_with_base (*p , 0 , p ,&personality);
        p = read_encoded_value_with_base (*p & 0x7F, 0, p + 1, &personality);
        cie_aug += 1;
      }
      else 
      {
          printf("here elseee \n");
          cie_aug +=1; 
      }
    } 
    
    fde_aug = fde + fde_id_offset + 4; //skip legnth and ID sections
    //fde_aug = fde + sizeof (*fde);
    printf("fde_aug1 %p \n",fde_aug);
    fde_aug += 2 * size_of_encoded_value (fde_encoding);
     printf("fde_aug2 %p \n",fde_aug);
    
    if (z_flag)
    {
      _uleb128_t i;
      fde_aug = read_uleb128 (fde_aug, &i);
      printf("fde_aug3 %p \n",fde_aug);
    }
    
    if (lsda_encoding != DW_EH_PE_omit)
    {
      test_Unwind_Ptr lsda;
       
      test_Unwind_Ptr base = (test_Unwind_Ptr) header.eh_frame;
      printf("base %p \n", (void *)base);
      fde_aug = read_encoded_value_with_base(lsda_encoding, base, fde_aug, &lsda);
      
      printf("generated lsda: %p \n", (void *)lsda);
      //context->lsda = (void *) lsda;
      
    }

}

const unsigned char *
test_extract_cie_info (const struct test_dwarf_cie *cie, struct test_Unwind_Context *context,
    test_Unwind_FrameState *fs)
{

}

test_Unwind_Reason_Code
test_uw_frame_state_for (struct test_Unwind_Context *context, test_Unwind_FrameState *fs)
{
    const struct test_dwarf_fde *fde;
    const struct test_dwarf_cie *cie;

    memset (fs, 0, sizeof (*fs));
    context->args_size = 0;
    context->lsda = 0;

    if (context->ra == 0)
        return _URC_END_OF_STACK;
    
    fde = find_fde(context->ra, context->bases);

    if (fde == NULL)
        return _URC_END_OF_STACK;

    fs->pc = context->bases.func;
    cie = test_get_cie (fde);

}

#ifdef __cplusplus
}
#endif