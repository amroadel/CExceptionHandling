#include "throw.h"
//#include "../test-unwind-eh.h"
#ifdef TEST_UNWIND
#include "read_elf.h"
#endif

extern char __executable_start;

int main(int argc, char *argv[])
{
    #ifdef TEST_UNWIND
    #include "read_elf.h"
    unsigned char *ptr2 =read_elf(argc, argv, "eh_frame_hdr");
    const unsigned char *eh_hdr = (const unsigned char *)((unsigned long)ptr2 + (unsigned long)&__executable_start);
    init_eh_frame_hdr(eh_hdr, (const unsigned char *)&__executable_start);
    #endif

    seppuku();
    return 0;
}

