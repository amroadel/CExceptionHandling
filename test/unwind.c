#include "stdlib.h"
#include "stdio.h"
#include "../test-unwind-pe.h"
#include "../test-unwind-eh.h"
#include "../test-unwind.h"
#include "read_elf.h"

extern char __executable_start;
extern char __etext;

struct test_Unwind_Context{
    void *ra;
    void *base;
    void *lsda;
};

void foo()
{
    struct test_Unwind_Context context;
    void *ra;

    void *bp = NULL;
    // asm ("movq %%rbp, %0\n"
    //      : "=r" (bp));
    bp = __builtin_frame_address(0);
    printf("%p\n", bp);
    ra = __builtin_extract_return_addr (__builtin_return_address (0));
    printf("%p\n", ra);
    printf("%lx\n\n", *((long *)bp));

    while (bp !=  0) {
        printf("%p\n", bp);
        ra = (void *) *((long *)bp + 1);
        bp = (void *) *((long *)bp);
        context.ra = ra;
        printf("%p\n", context.ra);
        find_fde(context.ra);
        printf("%p\n\n", bp);
    } ;
}

void bar1() {
    foo();
}

void bar2() {
    bar1();
}

void bar3() {
    bar2();
}


int main(int argc, char *argv[]) {
    printf("0x%lx\n", (unsigned long)&__executable_start);
    // void * b = sbrk (0);
    // printf ("brk(NULL): %p\n",b);
    // printf ("Hello World!: %p\n",main);

    unsigned char *ptr2 =read_elf(argc, argv);
    printf("ptr is %p\n", ptr2);
    // const unsigned char *ptr = &__executable_start;
    // printf("ptr is %p\n", ptr);
    // const unsigned char *eh_hdr = (const unsigned char *)((unsigned long)ptr2 + (unsigned long)&__executable_start);
    // printf("ptr is %p\n", eh_hdr);
    // for (int i = 0; i < 180; i++)
    //     printf("%02x ", *eh_hdr++);
    const unsigned char *eh_hdr = (const unsigned char *)((unsigned long)ptr2 + (unsigned long)&__executable_start);
    printf("ptr is %p\n", eh_hdr);
    printf("\n");
    init_eh_frame_hdr(eh_hdr);

    bar3();
    return 0;
}