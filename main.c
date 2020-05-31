#include <stdio.h>
#include <unistd.h>


extern char __executable_start;
extern char __etext;

int main ()
{

        printf ("Hello World!: %p\n",main);
        void * b = sbrk (0); //the end of data section
        printf ("brk(NULL): %p\n",b);
        printf("0x%lx\n", (unsigned long)&__executable_start); //begining of exe
        printf("0x%lx\n", (unsigned long)&__etext); //The end of the text section
        return 0;
}