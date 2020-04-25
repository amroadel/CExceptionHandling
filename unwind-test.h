#ifndef UNWIND_TEST
#define UNWIND_TEST

#ifdef __cplusplus
extern "C" {
#endif

#define NULL ((void*) 0) // this should be defined in std

typedef unsigned test_Unwind_Exception_Class __attribute__((__mode__(__DI__)));
typedef unsigned test_Unwind_Word __attribute__((__mode__(__DI__)));
typedef unsigned test_Unwind_Sword __attribute__((__mode__(__DI__)));
typedef unsigned test_Unwind_Ptr __attribute__((__mode__(__pointer__)));

// Data types
/*  Reason codes are used to indicate failures or action results  */
typedef enum {
    _URC_NO_REASON = 0,
    _URC_FOREIGN_EXCEPTION_CAUGHT = 1,
    _URC_FATAL_PHASE2_ERROR = 2,
    _URC_FATAL_PHASE1_ERROR = 3,
    _URC_NORMAL_STOP = 4,
    _URC_END_OF_STACK = 5,
    _URC_HANDLER_FOUND = 6,
    _URC_INSTALL_CONTEXT = 7,
    _URC_CONTINUE_UNWIND = 8
} test_Unwind_Reason_Code;

/*  Action was originally and int and separate defines  */
typedef enum {
    _UA_SEARCH_PHASE = 1,
    _UA_CLEANUP_PHASE = 2,
    _UA_HANDLER_FRAME = 4,
    _UA_FORCE_UNWIND = 8,
    _UA_END_OF_STACK = 16    
} test_Unwind_Action;

struct test_Unwind_Exception;

typedef void (*test_Unwind_Exception_Cleanup_Fn)
    (test_Unwind_Reason_Code,
    struct test_Unwind_Exception *);

struct test_Unwind_Exception {
    test_Unwind_Exception_Class exception_class;
    test_Unwind_Exception_Cleanup_Fn exception_cleanup;
    test_Unwind_Word private_1;
    test_Unwind_Word private_2;
};

struct test_Unwind_Context;

// Routines
test_Unwind_Reason_Code
test_Unwind_RaiseException(struct test_Unwind_Exception *exception_object);

typedef test_Unwind_Reason_Code (*test_Unwind_Stop_Fn)
    (int, test_Unwind_Action, test_Unwind_Exception_Class,
    struct test_Unwind_Exception *, struct test_Unwind_Context *, void *);

test_Unwind_Reason_Code
test_Unwind_ForcedUnwind(struct test_Unwind_Exception *, test_Unwind_Stop_Fn, void *);

void
test_Unwind_Resume(struct test_Unwind_Exception *);

void
test_Unwind_DeleteException(struct test_Unwind_Exception *);

test_Unwind_Word
test_Unwind_GetGR(struct test_Unwind_Context *, int);

void
test_Unwind_SetGR(struct test_Unwind_Context *, int, test_Unwind_Word);

test_Unwind_Ptr
test_Unwind_GetIP(struct test_Unwind_Context *);

void
test_Unwind_SetIP(struct test_Unwind_Context *, test_Unwind_Ptr);

test_Unwind_Word
test_Unwind_GetCFA(struct test_Unwind_Context *);

void *
test_Unwind_GetLanguageSpecificData(struct test_Unwind_Context *);

test_Unwind_Ptr
test_Unwind_GetRegionStart(struct test_Unwind_Context *);

typedef test_Unwind_Reason_Code (*test_Unwind_Personality_Fn)
    (int, test_Unwind_Action, test_Unwind_Exception_Class,
    struct test_Unwind_Exception *, struct test_Unwind_Context *);
    
void
test_Unwind(struct test_Unwind_Context *);

#ifdef __cplusplus
}
#endif

#endif // UNWIND_TEST