#include <stdio.h>
#include "throw.h"

int global_int = 0;

struct RAII {
    int i;
    RAII(int i) : i(i) { printf("RAII object %i has been built\n", i); }
    ~RAII() { printf("RAII object %i has been destroyed\n", i); }
};

struct Fake_Exception {};

void raise() {
    Exception e;
    e.num = 22;
    int i = 222;
    throw i;
}

void try_but_dont_catch() {
    RAII x(global_int++);

    try {
        raise();
    } catch(Fake_Exception&) {
        printf("Caught a Fake_Exception!\n");
    }

    printf("try_but_dont_catch handled the exception\n");
}

void catchit() {
    try {
        try_but_dont_catch();
    } catch(Fake_Exception&) {
        printf("Caught a Fake_Exception!\n");
    } catch(Exception) {
        printf("Caught an Exception!\n");
    } catch(int i) {
        printf("Exception int %i\n", i);
    }

    printf("catchit handled the exception\n");
}

extern "C" {
    void seppuku() {
        char str[] = "some random string";

        try {
            try_but_dont_catch();
        } catch(Fake_Exception&) {
            printf("Caught a Fake_Exception!\n");
        } catch(Exception& e) {
            printf("Caught an Exception!\n");
            printf("Exception num %i\n", e.num);
        } catch(int &i) {
            printf("Caught an int!\n");
            printf("Exception int %i\n", i);
        }
        printf("seppuku handled the exception\n");
        
        catchit();
    }
}

