#include <stdio.h>
#include <stdlib.h>
#include <typeinfo>
#include "test-unwind.h"
// #include "test-unwind-eh.h"
#include "test-unwind-pe.h"
// #include "test-unwind-fde.h"

#ifndef PROBE2
#define PROBE2(name, arg1, arg2)
#endif

// See below for C++
#ifndef _GLIBCXX_NOTHROW
# ifndef __cplusplus
#  define _GLIBCXX_NOTHROW __attribute__((__nothrow__))
# endif
#endif

// TODO: these should be in stdint
typedef unsigned char uint8_t;
typedef unsigned long uint64_t;
typedef unsigned long uintptr_t;
typedef signed int int32_t;

namespace __cxxabiv1 {
    struct __class_type_info {
        virtual void foo() {}
    } ti;
}

extern "C" {

typedef void (*unexpected_handler)(void);
typedef void (*terminate_handler)(void);

struct __cxa_exception { 
	std::type_info *	exceptionType;
	void (*exceptionDestructor) (void *); 
	unexpected_handler	unexpectedHandler;
	terminate_handler	terminateHandler;
	__cxa_exception *	nextException;

	int			handlerCount;
	int			handlerSwitchValue;
	const char *		actionRecord;
	const char *		languageSpecificData;
	void *			catchTemp;
	void *			adjustedPtr;

	test_Unwind_Exception	unwindHeader;
};

/*struct testtest_Unwind_Context{
    void *ra;
    void *base;
    void *lsda;
};*/

// Each thread in a C++ program has access to a __cxa_eh_globals object.
struct __cxa_eh_globals
{
  __cxa_exception *caughtExceptions;
  unsigned int uncaughtExceptions;

};

// Single-threaded fallback buffer.
__cxa_eh_globals eh_globals;

// The __cxa_eh_globals for the current thread can be obtained by using
// either of the following functions.  The "fast" version assumes at least
// one prior call of __cxa_get_globals has been made from the current
// thread, so no initialization is necessary.
__cxa_eh_globals*
__cxa_get_globals_fast() _GLIBCXX_NOTHROW
{ return &eh_globals; }

__cxa_eh_globals*
__cxa_get_globals() _GLIBCXX_NOTHROW
{ return &eh_globals; }

// Acquire the C++ exception header from the generic exception header.
inline __cxa_exception *
__get_exception_header_from_ue (struct test_Unwind_Exception *exc)
{
  return reinterpret_cast<__cxa_exception *>(exc + 1) - 1;
}

inline void*
__gxx_caught_object(struct test_Unwind_Exception* eo)
{
  // Bad as it looks, this actually works for dependent exceptions too.
  __cxa_exception* header = __get_exception_header_from_ue (eo);
  return header->adjustedPtr;
}

/********************************************************************************/

void* __cxa_allocate_exception(size_t thrown_size) noexcept
{
    void *ret;

    thrown_size += sizeof(__cxa_exception);
    ret = malloc (thrown_size);
    if (!ret)
        exit(0);

    return (void *)((char *)ret + sizeof(__cxa_exception));
}

void __cxa_free_exception(void *thrown_exception) noexcept
{
    char *ptr = (char *) thrown_exception - sizeof (__cxa_exception);
    free (ptr);
}

void __cxa_throw(void* thrown_exception,
                 std::type_info *tinfo,
                 void (*dest)(void*))
{
    __cxa_exception *header = ((__cxa_exception *) thrown_exception - 1);
    header->exceptionType = tinfo;
    header->exceptionDestructor = dest;
    test_Unwind_RaiseException(&header->unwindHeader);

    printf("no handler found, terminate!\n");
    exit(0);
}

void * __cxa_begin_catch(void *exc_obj_in)
{
    printf("begin FTW\n");
    test_Unwind_Exception *exceptionObject
    = reinterpret_cast <test_Unwind_Exception *>(exc_obj_in);
    __cxa_eh_globals *globals = __cxa_get_globals ();
    __cxa_exception *prev = globals->caughtExceptions;
    __cxa_exception *header = __get_exception_header_from_ue (exceptionObject);
    void* objectp;

    int count = header->handlerCount;
    // Count is less than zero if this exception was rethrown from an
    // immediately enclosing region.
    if (count < 0)
        count = -count + 1;
    else
        count += 1;

    header->handlerCount = count;
    globals->uncaughtExceptions -= 1;

    if (header != prev)
    {
        header->nextException = prev;
        globals->caughtExceptions = header;
    }

    objectp = __gxx_caught_object(exceptionObject);
    PROBE2 (catch, objectp, header->exceptionType);
    return objectp;

}

void __cxa_end_catch()
{
    printf("end FTW\n");
    __cxa_eh_globals *globals = __cxa_get_globals_fast ();
    __cxa_exception *header = globals->caughtExceptions;

    // A rethrow of a foreign exception will be removed from the
    // the exception stack immediately by __cxa_rethrow.
    if (!header)
        return;

    int count = header->handlerCount;
    if (count < 0)
    {
        // This exception was rethrown.  Decrement the (inverted) catch
        // count and remove it from the chain when it reaches zero.
        if (++count == 0)
            globals->caughtExceptions = header->nextException;
    }
    else if (--count == 0)
    {
      // Handling for this exception is complete.  Destroy the object.
      globals->caughtExceptions = header->nextException;
      test_Unwind_DeleteException (&header->unwindHeader);
      return;
    }
    else if (count < 0)
    {
        // A bug in the exception handling library or compiler.
        printf("no handler found, terminate!\n");
        exit(0);
    }
        
    header->handlerCount = count;
}




/**********************************************/

int readSLEB128(const uint8_t* data)
{
    uint64_t result = 0;
    uint64_t shift = 0;
    unsigned char byte;
    const uint8_t *p = data;

    do
    {
        byte = *p++;
        result |= static_cast<uint64_t>(byte & 0x7F) << shift;
        shift += 7;
    } while (byte & 0x80);

    if ((byte & 0x40) && (shift < (sizeof(result) << 3)))
        result |= static_cast<uint64_t>(~0) << shift;

    return static_cast<int>(result);
}




/**
 * The LSDA is a read only place in memory; we'll create a typedef for
 * this to avoid a const mess later on; LSDA_ptr refers to readonly and
 * &LSDA_ptr will be a non-const pointer to a const place in memory
  */
typedef const uint8_t* LSDA_ptr;
typedef uint64_t LSDA_line;

//This function receives a pointer the first byte to be decoded and the address of where to put the decoded value.
// const unsigned char *
// read_uleb128 (const unsigned char *p, uint64_t *val)
// {
//   uint64_t shift = 0;
//   uint64_t result = 0;
//   unsigned char byte;

//   do
//     {
//       byte = *p++; 
//       // Shifting the byte to the left and propagating the new byte to it (if there's any)
//       result |= ((uint64_t)byte & 0x7f) << shift;
//       shift += 7;
//     }
//   // if the 8th bit is 1, this means that there still another byte to be concatinated to the current byte (if zero, then decoding is done)
//   while (byte & 0x80);
//   *val = result;
//   return p;
// }

// This function recieves a pointer to a ttype entry and return the corrisponding type_info
// It's an implementation of three different functions from unwind-pe.h specific to our case and cannot be generalized for different encoding and size
const std::type_info *
get_ttype_entry (const uint8_t* entry)
{
    const int32_t *u = (const int32_t *) entry;
    unsigned long result;

    if (*u == 0)
        return NULL;

    result = *u + (unsigned long)u;
    result = *(unsigned long *)result;
    
    const std::type_info *tinfo = reinterpret_cast<const std::type_info *>(result);
    return tinfo;
}

struct LSDA_Header {
    /**
     * Read the LSDA table into a struct; advances the lsda pointer
     * as many bytes as read
     */
    LSDA_Header(LSDA_ptr *lsda) {
        const unsigned char *read_ptr = (const unsigned char *)*lsda;
        start_encoding = read_ptr[0];
        type_encoding = read_ptr[1];
        read_ptr += 2;
        read_ptr = read_uleb128(read_ptr, &type_table_offset);
        *lsda = (LSDA_ptr)read_ptr;
    }

    uint8_t start_encoding;
    uint8_t type_encoding;

    // This is the offset, from the end of the header, to the types table
    LSDA_line type_table_offset;
};

struct Call_Site_Header {
    // Same as other LSDA constructors
    Call_Site_Header(LSDA_ptr *lsda) {
        const unsigned char *read_ptr = (const unsigned char *)*lsda;
        encoding = read_ptr[0];
        read_ptr += 1;
        read_ptr = read_uleb128(read_ptr, &length);
        *lsda = (LSDA_ptr)read_ptr;
    }

    uint8_t encoding;
    LSDA_line length;
};

struct Call_Site {
    // Same as other LSDA constructors
    Call_Site(LSDA_ptr *lsda) {
        const unsigned char *read_ptr = (const unsigned char *)*lsda;
        read_ptr = read_uleb128(read_ptr, &start);
        read_ptr = read_uleb128(read_ptr, &len);
        read_ptr = read_uleb128(read_ptr, &lp);
        read_ptr = read_uleb128(read_ptr, &action);
        *lsda = (LSDA_ptr)read_ptr;
    }

    Call_Site() { }

    // Note start, len and lp would be void*'s, but they are actually relative
    // addresses: start and lp are relative to the start of the function, len
    // is relative to start
 
    // Offset into function from which we could handle a throw
    LSDA_line start;
    // Length of the block that might throw
    LSDA_line len;
    // Landing pad
    LSDA_line lp;
    // Offset into action table + 1 (0 means no action)
    // Used to run destructors
    LSDA_line action;

    bool has_landing_pad() const { return lp; }

    /**
     * Returns true if the instruction pointer for this call frame
     * (throw_ip) is in the range of the landing pad for this call
     * site; if true that means the exception was thrown from within
     * this try/catch block
     */
    bool valid_for_throw_ip(uintptr_t func_start, uintptr_t throw_ip) const
    {
        // Calculate the range of the instruction pointer valid for this
        // landing pad; if this LP can handle the current exception then
        // the IP for this stack frame must be in this range
        uintptr_t try_start = func_start + this->start;
        uintptr_t try_end = func_start + this->start + this->len;

        // Check if this is the correct LP for the current try block
        if (throw_ip < try_start) return false;
        if (throw_ip > try_end) return false;

        // The current exception was thrown from this landing pad
        return true;
    }
};

/**
 * A class to read the language specific data for a function
  */
struct LSDA
{
    LSDA_Header header;

    // The types_table_start holds all the types this stack frame
    // could handle (this table will hold pointers to struct
    // type_info so this is actually a pointer to a list of ptrs
    const void** types_table_start;

    // With the call site header we can calculate the lenght of the
    // call site table
    Call_Site_Header cs_header;

    // A pointer to the start of the call site table
    const LSDA_ptr cs_table_start;

    // A pointer to the end of the call site table
    const LSDA_ptr cs_table_end;

    // A pointer to the start of the action table, where an action is
    // defined for each call site
    const LSDA_ptr action_tbl_start;

    LSDA(LSDA_ptr raw_lsda) :
        // Read LSDA header for the LSDA, advance the ptr
        header(&raw_lsda),

        // Get the start of the types table (it's actually the end of the
        // table, but since the action index will hold a negative index
        // for this table we can say it's the beginning
        types_table_start( (const void**)((uint8_t*)raw_lsda + header.type_table_offset) ),

        // Read the LSDA CS header
        cs_header(&raw_lsda),

        // The call site table starts immediatelly after the CS header
        cs_table_start(raw_lsda),

        // Calculate where the end of the LSDA CS table is
        cs_table_end((const LSDA_ptr)((uint8_t*)(raw_lsda) + cs_header.length)),

        // Get the start of action tables
        action_tbl_start( cs_table_end )
    {
    }
   

    Call_Site next_cs_entry;
    LSDA_ptr next_cs_entry_ptr;

    const Call_Site* next_call_site_entry(bool start=false)
    {
        if (start) next_cs_entry_ptr = cs_table_start;

        // If we went over the end of the table return NULL
        if (next_cs_entry_ptr >= cs_table_end)
            return NULL;

        // Copy the call site table and advance the cursor by sizeof(Call_Site).
        // We need to copy the struct here because there might be alignment
        // issues otherwise
        next_cs_entry = Call_Site(&next_cs_entry_ptr);

        return &next_cs_entry;
    }


    /**
     * Returns a pointer to the action entry for a call site entry or
     * null if the CS has no action
     */
    const LSDA_ptr get_action_for_call_site(const Call_Site *cs) const
    {
        if (cs->action == 0) return NULL;

        const size_t action_offset = cs->action - 1;
        return this->action_tbl_start + action_offset;
    }


    /**
     * An entry in the action table
     */
    struct Action {
        // An index into the types table
        int type_index;

        // Offset for the next action, relative from this byte (this means
        // that the next action will begin exactly at the address of
        // &next_offset - next_offset itself
        int next_offset;

        // A pointer to the raw action, which we need to get the next
        // action:
        //   next_action_offset = raw_action_ptr[1]
        //   next_action_ptr = &raw_action_ptr[1] + next_action_offset
        LSDA_ptr raw_action_ptr;

    } current_action;


    /**
     * Gets the first action for a specific call site
     */
    const Action* get_first_action_for_cs(const Call_Site *cs)
    {
        // The call site may have no associated action (in that case
        // it should be a cleanup)
        if (cs->action == 0) return NULL;

        // The action in the CS is 1 based: 0 means no action and
        // 1 is the element 0 on the action table
        const size_t action_offset = cs->action - 1;
        LSDA_ptr action_raw = this->action_tbl_start + action_offset;

        current_action.type_index = action_raw[0];
        current_action.next_offset = readSLEB128( &action_raw[1] );
        current_action.raw_action_ptr = &action_raw[0];

        return &current_action;
    }

    /**
     * Gets the next action, if any, for a CS (after calling
     * get_first_action_for_cs)
     */
    const Action* get_next_action() {
        // If the current_action is the last one then the
        // offset for the next one will be 0
        if (current_action.next_offset == 0) return NULL;

        // To move to the next action we must use raw_action_ptr + 1
        // because the offset is from the next_offset place itself and
        // not from the start of the struct:
        LSDA_ptr action_raw = current_action.raw_action_ptr + 1 +
                                        current_action.next_offset;

        current_action.type_index = action_raw[0];
        current_action.next_offset = readSLEB128( &action_raw[1] );
        current_action.raw_action_ptr = &action_raw[0];

        return &current_action;
    }

    /**
     * Returns the type from the types table defined for an action
     */
    const std::type_info* get_type_for(const Action* action) const
    {
        // The index starts at the end of the types table
        int idx = -1 * action->type_index;
        const uint8_t* catch_type_info = ((const uint8_t*)this->types_table_start) + (idx * 4);
        //return 0;
        return get_ttype_entry(catch_type_info);;
    }
};


/**********************************************/


bool can_handle(const std::type_info *thrown_exception,
                const std::type_info *catch_type)
{
    // If the catch has no type specifier we're dealing with a catch(...)
    // and we can handle this exception regardless of what it is
    if (not catch_type) return true;

    // Naive type comparisson: only check if the type name is the same
    // This won't work with any kind of inheritance
    if (thrown_exception->name() == catch_type->name())
        return true;

    // If types don't match just don't handle the exception
    return false;
}


test_Unwind_Reason_Code
    run_landing_pad(
                 test_Unwind_Exception* unwind_exception,
                 test_Unwind_Context* context,
                 int exception_type_idx,
                 uintptr_t lp_address)
{
    int r0 = __builtin_eh_return_data_regno(0);
    int r1 = __builtin_eh_return_data_regno(1);

    test_Unwind_SetGR(context, r0, (uintptr_t)(unwind_exception));
    test_Unwind_SetGR(context, r1, (uintptr_t)(exception_type_idx));
    test_Unwind_SetIP(context, lp_address);

    return _URC_INSTALL_CONTEXT;
}


test_Unwind_Reason_Code __gxx_personality_v0 (
                             int version,
                             test_Unwind_Action actions,
                             uint64_t exceptionClass,
                             test_Unwind_Exception* unwind_exception,
                             test_Unwind_Context* context)
{
    // Calculate what the instruction pointer was just before the
    // exception was thrown for this stack frame
    uintptr_t throw_ip = test_Unwind_GetIP(context) - 1;

    // Get a ptr to the start of the function for this stack frame;
    // this is needed because a lot of the addresses in the LSDA are
    // actually offsets from func_start
    uintptr_t func_start = test_Unwind_GetRegionStart(context);

    // Get a pointer to the type_info of the exception being thrown
    __cxa_exception *exception_header =(__cxa_exception*)(unwind_exception+1)-1;
    std::type_info *thrown_exception_type = exception_header->exceptionType;

    // Get a pointer to the raw memory address of the LSDA
    LSDA_ptr raw_lsda = (LSDA_ptr) test_Unwind_GetLanguageSpecificData(context);

    // Create an object to hide some part of the LSDA processing
    LSDA lsda(raw_lsda);

    //test
    // struct testtest_Unwind_Context *context2;
    // struct test_dwarf_eh_bases *bases;
    // const unsigned char* fde = (const unsigned char*)find_fde((void *)(throw_ip + 1), bases);
    // //add_lsda(fde, context2);
    
    // testtest_Unwind_FrameState *fs;
    // testtest_Unwind_Reason_Code code;
    // //const unsigned char* fde = find_fde((void *)(throw_ip + 1));
    // //add_lsda(fde, context2);
    // code = test_uw_frame_state_for(context2, fs);
    // void *lsdaa = (void *) test_Unwind_GetLanguageSpecificData(context);
    // printf("real lsda: %p\n", lsdaa);
    // //printf("lsda2: %p\n", lsda);

    // Go through each call site in this stack frame to check whether
    // the current exception can be handled here
    for(const Call_Site *cs = lsda.next_call_site_entry(true);
            cs != NULL;
            cs = lsda.next_call_site_entry())
    {
        // If there's no landing pad we can't handle this exception
        if (not cs->has_landing_pad()) continue;

        // Calculate the range of the instruction pointer valid for this
        // landing pad; if this LP can handle the current exception then
        // the IP for this stack frame must be in this range
        if (not cs->valid_for_throw_ip(func_start, throw_ip)) continue;

        // Iterate all the actions for this call site
        for (const LSDA::Action* action = lsda.get_first_action_for_cs(cs);
                action != NULL;
                action = lsda.get_next_action())
        {
            if (action->type_index == 0)
            {
                // If there is an action entry but it doesn't point to any
                // type, it means this is actually a cleanup block and we
                // should run it anyway
                //
                // Of course the cleanup should only run on the cleanup phase
                if (actions & _UA_CLEANUP_PHASE)
                {
                    return run_landing_pad(unwind_exception, context,
                                    action->type_index, func_start + cs->lp);
                }
            } else {
                // Get the types this action can handle
                const std::type_info *catch_type = lsda.get_type_for(action);

                if (can_handle(thrown_exception_type, catch_type)) // order reversed in original
                {
                    // If we are on search phase, tell test_Unwind_ we can handle this one
                    if (actions & _UA_SEARCH_PHASE) return _URC_HANDLER_FOUND;

                    // If we are not on search phase then we are on _UA_CLEANUP_PHASE
                    // and we need to install the context
                    return run_landing_pad(unwind_exception, context,
                                    action->type_index, func_start + cs->lp);
                }
            }
        }
    }

    return _URC_CONTINUE_UNWIND;
}

}