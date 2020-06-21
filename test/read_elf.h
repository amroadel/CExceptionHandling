#ifndef __READ_ELF_H__
#define __READ_ELF_H__

#include <stdio.h>
#include <xmmintrin.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>
#include <elf.h>


typedef struct elf_file {
    FILE *file;         // file stream
    Elf64_Ehdr *ehdr64; // elf header table
    Elf64_Phdr *phdr64; // program header table
    Elf64_Shdr *shdr64; // section header table
} elf_file_t;


/* retrieve elf file handle */
elf_file_t *re_create_handle(const char *pathname);

/* retrive elf file headers */
Elf64_Ehdr *re_get_elf_header(elf_file_t *hdl);
Elf64_Phdr *re_get_program_header(elf_file_t *hdl);
Elf64_Shdr *re_get_section_header(elf_file_t *hdl);

/* clean the elf file handle */
void re_free_handle(elf_file_t *hdl);

#define FILENAME_SIZE 9216

struct argument {
    char file[FILENAME_SIZE];
    bool ehdr_flag;
    bool phdr_flag;
    bool shdr_flag;
};

char* get_sectionHeader_string_table(Elf64_Ehdr *hdr);
static Elf64_Addr dump_section_header(Elf64_Ehdr *ehdr, Elf64_Shdr *shdr, const char * section);
unsigned char *read_elf(int argc, char **argv, const char * section);


#endif /* __READ_ELF_H__ */

