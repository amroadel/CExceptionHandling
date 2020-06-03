#ifndef TEST_UNWIND
#define TEST_UNWIND

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned test_Unwind_Exception_Class __attribute__((__mode__(__DI__)));
typedef unsigned test_Unwind_Word __attribute__((__mode__(__DI__)));
typedef signed test_Unwind_Sword __attribute__((__mode__(__DI__)));
typedef unsigned test_Unwind_Ptr __attribute__((__mode__(__pointer__)));

/* Data types */
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

struct test_Unwind_Context;

/* Routines */
test_Unwind_Reason_Code
test_Unwind_RaiseException(struct test_Unwind_Exception *exc);

typedef test_Unwind_Reason_Code (*test_Unwind_Stop_Fn)
    (int, test_Unwind_Action, test_Unwind_Exception_Class,
    struct test_Unwind_Exception *, struct test_Unwind_Context *, void *);

test_Unwind_Reason_Code
test_Unwind_ForcedUnwind(struct test_Unwind_Exception *, test_Unwind_Stop_Fn stop, void *stop_argument);

void
test_Unwind_Resume(struct test_Unwind_Exception *exc);

void
test_Unwind_DeleteException(struct test_Unwind_Exception *exc);

test_Unwind_Word
test_Unwind_GetGR(struct test_Unwind_Context *context, int index);

void
test_Unwind_SetGR(struct test_Unwind_Context *context, int index, test_Unwind_Word val);

test_Unwind_Ptr
test_Unwind_GetIP(struct test_Unwind_Context *context);

void
test_Unwind_SetIP(struct test_Unwind_Context *context, test_Unwind_Ptr val);

test_Unwind_Word
test_Unwind_GetCFA(struct test_Unwind_Context *context);

test_Unwind_Ptr
test_Unwind_GetLanguageSpecificData(struct test_Unwind_Context *context);

test_Unwind_Ptr
test_Unwind_GetRegionStart(struct test_Unwind_Context *context);

test_Unwind_Ptr
test_Unwind_GetTextRelBase(struct test_Unwind_Context *context);

test_Unwind_Ptr
test_Unwind_GetDataRelBase(struct test_Unwind_Context *context);

typedef test_Unwind_Reason_Code (*test_Unwind_Personality_Fn)
    (int, test_Unwind_Action, test_Unwind_Exception_Class,
    struct test_Unwind_Exception *, struct test_Unwind_Context *);

#ifdef __cplusplus
}
#endif

#endif /* TEST_UNWIND */