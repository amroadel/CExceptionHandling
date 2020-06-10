#include "throw.h"
#include "../test-unwind-eh.h"
#include "read_elf.h"

extern char __executable_start;

int main(int argc, char *argv[])
{
    unsigned char *ptr2 =read_elf(argc, argv);
    const unsigned char *eh_hdr = (const unsigned char *)((unsigned long)ptr2 + (unsigned long)&__executable_start);
    init_eh_frame_hdr(eh_hdr, (const unsigned char *)&__executable_start);
    seppuku();
    return 0;
}

