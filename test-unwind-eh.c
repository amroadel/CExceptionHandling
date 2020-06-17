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
//Removed frame_state struct from here to fde.h

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

/* Signal frame context.  */
#define SIGNAL_FRAME_BIT ((~(test_Unwind_Word) 0 >> 1) + 1)

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

inline test_Unwind_Word
test_Unwind_IsSignalFrame (struct test_Unwind_Context *context)
{
    return (context->flags & SIGNAL_FRAME_BIT) ? 1 : 0;
}

void
test_execute_cfa_program (const unsigned char *insn_ptr,
		     const unsigned char *insn_end,
		     struct test_Unwind_Context *context,
		     test_Unwind_FrameState *fs)
{


}

const unsigned char *
test_extract_cie_info (const struct test_dwarf_cie *cie, struct test_Unwind_Context *context,
    test_Unwind_FrameState *fs)
{

    const unsigned char *aug = cie->augmentation;
    const unsigned char *p = aug + strlen ((const char *)aug) + 1;
    const unsigned char *ret = NULL;
    _uleb128_t utmp;
    _sleb128_t stmp;

    /* g++ v2 "eh" has pointer immediately following augmentation string,
        so it must be handled first.  */
    if (aug[0] == 'e' && aug[1] == 'h')
    {
        //fs->eh_ptr = read_pointer (p); //TODO: Check for read_pointer()
        fs->eh_ptr = (void *)p;
        p += sizeof (void *);
        aug += 2;
    }

  /* After the augmentation resp. pointer for "eh" augmentation
     follows for CIE version >= 4 address size byte and
     segment size byte.  */
  if (cie->version >= 4)
    {
      if (p[0] != sizeof (void *) || p[1] != 0)
	    return NULL;
      p += 2;
    }
    /* Immediately following this are the code and
        data alignment and return address column.  */
    p = read_uleb128 (p, &utmp);
    fs->code_align = (test_Unwind_Word)utmp;
    p = read_sleb128 (p, &stmp);
    fs->data_align = (test_Unwind_Sword)stmp;
    if (cie->version == 1)
        fs->retaddr_column = *p++;
    else
        {
        p = read_uleb128 (p, &utmp);
        fs->retaddr_column = (test_Unwind_Word)utmp;
        }
    fs->lsda_encoding = DW_EH_PE_omit;

    /* If the augmentation starts with 'z', then a uleb128 immediately
        follows containing the length of the augmentation field following
        the size.  */
    if (*aug == 'z')
        {
        p = read_uleb128 (p, &utmp);
        ret = p + utmp;

        fs->saw_z = 1;
        ++aug;
        }

    /* Iterate over recognized augmentation subsequences.  */
    while (*aug != '\0')
        {
        /* "L" indicates a byte showing how the LSDA pointer is encoded.  */
        if (aug[0] == 'L')
        {
        fs->lsda_encoding = *p++;
        aug += 1;
        }

        /* "R" indicates a byte indicating how FDE addresses are encoded.  */
        else if (aug[0] == 'R')
        {
        fs->fde_encoding = *p++;
        aug += 1;
        }

        /* "P" indicates a personality routine in the CIE augmentation.  */
        else if (aug[0] == 'P')
        {
        test_Unwind_Ptr personality;

        p = read_encoded_value (context, *p, p + 1, &personality);
        fs->personality = (test_Unwind_Personality_Fn) personality;
        aug += 1;
        }

        /* "S" indicates a signal frame.  */
        else if (aug[0] == 'S')
        {
        fs->signal_frame = 1;
        aug += 1;
        }
        /* aarch64 B-key pointer authentication.  */
        else if (aug[0] == 'B')
        {
        aug += 1;
        }

        /* Otherwise we have an unknown augmentation string.
        Bail unless we saw a 'z' prefix.  */
        else
        return ret;
        }

    return ret ? ret : p;
}

test_Unwind_Reason_Code
test_uw_frame_state_for (struct test_Unwind_Context *context, test_Unwind_FrameState *fs)
{
    const struct test_dwarf_fde *fde;
    const struct test_dwarf_cie *cie;
    const unsigned char *aug, *insn, *end;

    memset (fs, 0, sizeof (*fs));
    context->args_size = 0;
    context->lsda = 0;

    if (context->ra == 0)
        return _URC_END_OF_STACK;
    
    fde = find_fde(context->ra, &context->bases);

    if (fde == NULL)
        return _URC_END_OF_STACK;

    fs->pc = context->bases.func;
    cie = test_get_cie (fde);
    insn = test_extract_cie_info (cie, context, fs);
    if (insn == NULL)
        /* CIE contained unknown augmentation.  */
        return _URC_FATAL_PHASE1_ERROR;

    /* First decode all the insns in the CIE.  */
    end = (const unsigned char *) test_next_fde ((const struct test_dwarf_fde *) cie);
    //test_execute_cfa_program (insn, end, context, fs);

    /* Locate augmentation for the fde.  */
    aug = (const unsigned char *) fde + sizeof (*fde);
    aug += 2 * size_of_encoded_value (fs->fde_encoding);
    insn = NULL;

    if (fs->saw_z)
    {
        _uleb128_t i;
        aug = read_uleb128 (aug, &i);
        insn = aug + i;
    }

    if (fs->lsda_encoding != DW_EH_PE_omit)
    {
        test_Unwind_Ptr lsda;

        aug = read_encoded_value (context, fs->lsda_encoding, aug, &lsda);
        context->lsda = (void *) lsda;
        printf("generated lsda: %p\n", lsda);
    }

    /* Then the insns in the FDE up to our target PC.  */
    if (insn == NULL)
        insn = aug;
    end = (const unsigned char *) test_next_fde (fde);
    //test_execute_cfa_program (insn, end, context, fs);

    return _URC_NO_REASON;

}
typedef union { test_Unwind_Ptr ptr; test_Unwind_Word word; } test_Unwind_SpTmp;
#define __builtin_dwarf_sp_column() 7 // this is in the case of x86_64 arch(our case). Other cases are found at unwind-pnacl.h

long
test_uw_install_context_1 (struct test_Unwind_Context *current, struct test_Unwind_Context *target)
{
    long i; 
    test_Unwind_SpTmp sp_slot; 
    
}
#ifdef __cplusplus
}
#endif