

#include "read_elf.h"



elf_file_t *re_create_handle(const char *pathname)
{
    elf_file_t *ret = NULL;

    elf_file_t *hdl = calloc(1, sizeof(elf_file_t));
    if (hdl == NULL)
        return NULL;

    hdl->file = fopen(pathname, "rb");
    if (hdl->file == NULL)
        goto clean_hdl;

    // retrieve elf file header
    hdl->ehdr64 = calloc(1, sizeof(Elf64_Ehdr));
    if (hdl->ehdr64 == NULL)
        goto clean_hdl_file;

    fread(hdl->ehdr64, 1, sizeof(Elf64_Ehdr), hdl->file);

    if (hdl->ehdr64->e_ident[EI_MAG0] != ELFMAG0 &&
            hdl->ehdr64->e_ident[EI_MAG1] != ELFMAG1 &&
            hdl->ehdr64->e_ident[EI_MAG2] != ELFMAG2 &&
            hdl->ehdr64->e_ident[EI_MAG3] != ELFMAG3)
        goto clean_hdl_file_ehdr;

    // retrieve section header
    // hdl->shdr64 = calloc(1, sizeof(Elf64_Shdr));
    hdl->shdr64 = calloc(1, hdl->ehdr64->e_shentsize * hdl->ehdr64->e_shnum);
    if (hdl->shdr64 == NULL)
        goto clean_hdl_file_ehdr_phdr_shdr;

    fseek(hdl->file, hdl->ehdr64->e_shoff, SEEK_SET);
//    fread(hdl->shdr64, 1, sizeof(Elf64_Shdr), hdl->file);
    fread(hdl->shdr64, 1, hdl->ehdr64->e_shentsize * hdl->ehdr64->e_shnum, hdl->file);

    // if everything succeed, return the new handle
    ret = hdl;
    goto success;

clean_hdl_file_ehdr_phdr_shdr:
    free(hdl->shdr64);
clean_hdl_file_ehdr_phdr:
    free(hdl->phdr64);
clean_hdl_file_ehdr:
    free(hdl->ehdr64);
clean_hdl_file:
    fclose(hdl->file);
clean_hdl:
    free(hdl);
success:
    return ret;
}



Elf64_Shdr *re_get_section_header(elf_file_t *hdl)
{
    return hdl->shdr64;
}


void re_free_handle(elf_file_t *hdl)
{
    fclose(hdl->file);
    free(hdl->shdr64);
    free(hdl);
}


elf_file_t *hdl;




/*
*This function loads the section table string table into memory and returns a pointer to the start of the string table
*/
char* get_sectionHeader_string_table(Elf64_Ehdr *hdr)
{   //shstrndx is the index of the section in the section table
    int index=  hdl->ehdr64->e_shstrndx;
    //This gets the section that stores the string table
    const Elf64_Shdr* shstr = &(hdl->shdr64)[index];
    //The offset of the section in the file on the disk
    Elf64_Off strtab_offset = shstr->sh_offset;

    //So far only the headers were loaded into memory, now we need to load the string table section itself into memory
    fseek(hdl->file,strtab_offset, SEEK_SET);//to start reading from the appropriate section 
    char * String_Table = malloc(sizeof(char)*shstr->sh_size);
    fread(String_Table, sizeof(char),shstr->sh_size, hdl->file);


    return String_Table;
}



/*  Display the section header  */
static Elf64_Addr dump_section_header(Elf64_Ehdr *ehdr, Elf64_Shdr *shdr )
{
    Elf64_Addr  eh_frame_v_address;
    char * stringTable= get_sectionHeader_string_table(ehdr);
    unsigned int i;
    for (i = 0; i < ehdr->e_shnum; i++)
    {
        const Elf64_Shdr *ite = &shdr[i];
        char * entry = stringTable + ite->sh_name;
        //This gets the offset of the string we want, the string is null-terminated so printf will detect the end of the string

        if(strcmp(entry, ".eh_frame_hdr")==0)
           {    
               eh_frame_v_address= ite->sh_addr;
               //printf("%ld\n",eh_frame_v_address);
          }
    }
    return eh_frame_v_address;

}
 

unsigned char *read_elf(int argc, char **argv)
{
    struct argument args;
     strncpy(args.file, argv[optind], FILENAME_SIZE);
    hdl = re_create_handle(args.file);
    if (hdl == NULL)
        return NULL;

    /*  retreive pointers for each elf file sections  */
    Elf64_Shdr *shdr = re_get_section_header(hdl);
    Elf64_Ehdr *ehdr = hdl->ehdr64;

    unsigned char *eh_frame_v_address = (unsigned char *)dump_section_header(ehdr, shdr);

    re_free_handle(hdl);

    return eh_frame_v_address;
}

