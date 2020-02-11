
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// definition for a name space used in the ABI from cxxabi.h
namespace __cxxabiv1 {
    // mock definition for the clasa implemented in cxxabi.h. Originally it inherits from std::type_info
    struct __class_type_info {
        virtual void foo() {}
    } ti;
}

// the exception is being allocated as an array of this size
#define EXCEPTION_BUFF_SIZE 255
char exception_buff[EXCEPTION_BUFF_SIZE];

extern "C" {

void* __cxa_allocate_exception(size_t thrown_size)
{
    printf("alloc ex %i\n", thrown_size);
    if (thrown_size > EXCEPTION_BUFF_SIZE) printf("Exception too big");
    return &exception_buff;
}

void __cxa_free_exception(void *thrown_exception);


#include <unwind.h>
#include <typeinfo>

// type definitions for function pointers that return void and takes void as a parameter. The original definition doesn't take any parameters in exception headeer under std
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

	_Unwind_Exception	unwindHeader;
};

void __cxa_throw(void* thrown_exception,
                 std::type_info *tinfo,
                 void (*dest)(void*))
{
    printf("__cxa_throw called\n");

    // thrown_exception pointer is being decremented for some reason. Need to check why?? TODO
    __cxa_exception *header = ((__cxa_exception *) thrown_exception - 1);

    _Unwind_RaiseException(&header->unwindHeader);

    // __cxa_throw never returns
    printf("no one handled __cxa_throw, terminate!\n");
    exit(0);
}


void __cxa_begin_catch()
{
    printf("begin FTW\n");
}

void __cxa_end_catch()
{
    printf("end FTW\n");
}




/**********************************************/

/**
 * The LSDA is a read only place in memory; we'll create a typedef for
 * this to avoid a const mess later on; LSDA_ptr refers to readonly and
 * &LSDA_ptr will be a non-const pointer to a const place in memory
 */

typedef const _uleb128_t* LSDA_ptr;
typedef _uleb128_t LSDA_line;

// __uint128_t dec_uleb128_old(LSDA_ptr &lsda) {
//     LSDA_line result = 0;
//     uint8_t shift = 0;
//     uint8_t uleb_byte = 0;
//     int i = 0;
//     while (true) {
//         LSDA_ptr read_ptr = lsda;
//         uleb_byte = (uint8_t)read_ptr[0];
//         result |= (0x7f & uleb_byte) << shift;
//         lsda += sizeof(uint8_t);
//         printf("%i\n",read_ptr);
//         if ((uleb_byte >> 7) == 0)
//             break;
//         shift += 7;
//         i++;
//     }
//     return result;
// }

static const unsigned char *
dec_uleb128 (const unsigned char *p, _uleb128_t *val)
{
  unsigned int shift = 0;
  unsigned char byte;
  _uleb128_t result;
  result = 0;
  do
    {
      byte = *p++;
      result |= ((_uleb128_t)byte & 0x7f) << shift;
      shift += 7;
    }
  while (byte & 0x80);
  *val = result;
  return p;
}

struct LSDA_Header {
    /**
     * Read the LSDA table into a struct; advances the lsda pointer
     * as many bytes as read
     */
    LSDA_Header(LSDA_ptr *lsda) {
        //LSDA_ptr read_ptr = *lsda;

        // Copy the LSDA fields
        // start_encoding = read_ptr[0];
        // type_encoding = read_ptr[1];
        // type_table_offset = read_ptr[2];

        const unsigned char *read_ptr = (const unsigned char *)*lsda;
        read_ptr = dec_uleb128(read_ptr, &start_encoding);
        read_ptr = dec_uleb128(read_ptr, &type_encoding);
        read_ptr = dec_uleb128(read_ptr, &type_table_offset);
        *lsda = (LSDA_ptr)read_ptr;

        // Advance the lsda pointer
        //*lsda = read_ptr + sizeof(LSDA_Header);
    }

    LSDA_line start_encoding;
    LSDA_line type_encoding;

    // This is the offset, from the end of the header, to the types table
    LSDA_line type_table_offset;
};

struct LSDA_CS_Header {
    // Same as other LSDA constructors
    LSDA_CS_Header(LSDA_ptr *lsda) {
        //LSDA_ptr read_ptr = *lsda;
        //encoding = read_ptr[0];
        //length = read_ptr[1];

        const unsigned char *read_ptr = (const unsigned char *)*lsda;
        read_ptr = dec_uleb128(read_ptr, &encoding);
        read_ptr = dec_uleb128(read_ptr, &length);
        *lsda = (LSDA_ptr)read_ptr;

        //*lsda = read_ptr + sizeof(LSDA_CS_Header);
    }

    LSDA_line encoding;
    LSDA_line length;
};

struct LSDA_CS {
    // Same as other LSDA constructors
    LSDA_CS(LSDA_ptr *lsda) {
        //LSDA_ptr read_ptr = *lsda;
        // start = read_ptr[0];
        // len = read_ptr[1];
        // lp = read_ptr[2];
        //action = read_ptr[3];

        const unsigned char *read_ptr = (const unsigned char *)*lsda;
        read_ptr = dec_uleb128(read_ptr, &start);
        read_ptr = dec_uleb128(read_ptr, &len);
        read_ptr = dec_uleb128(read_ptr, &lp);
        read_ptr = dec_uleb128(read_ptr, &action);
        *lsda = (LSDA_ptr)read_ptr;

        //*lsda = read_ptr + sizeof(LSDA_CS);
    }

    LSDA_CS() { }

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
    LSDA_CS_Header cs_header;

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
        types_table_start( (const void**)(raw_lsda + header.type_table_offset) ),

        // Read the LSDA CS header
        cs_header(&raw_lsda),

        // The call site table starts immediatelly after the CS header
        cs_table_start(raw_lsda),

        // Calculate where the end of the LSDA CS table is
        cs_table_end(raw_lsda + cs_header.length),

        // Get the start of action tables
        action_tbl_start( cs_table_end )
    {
    }
   

    LSDA_CS next_cs_entry;
    LSDA_ptr next_cs_entry_ptr;

    const LSDA_CS* next_call_site_entry(bool start=false)
    {
        if (start) next_cs_entry_ptr = cs_table_start;

        // If we went over the end of the table return NULL
        if (next_cs_entry_ptr >= cs_table_end)
            return NULL;

        // Copy the call site table and advance the cursor by sizeof(LSDA_CS).
        // We need to copy the struct here because there might be alignment
        // issues otherwise
        next_cs_entry = LSDA_CS(&next_cs_entry_ptr);

        return &next_cs_entry;
    }
};


/**********************************************/


_Unwind_Reason_Code __gxx_personality_v0 (
                             int version,
                             _Unwind_Action actions,
                             uint64_t exceptionClass,
                             _Unwind_Exception* unwind_exception,
                             _Unwind_Context* context)
{
    if (actions & _UA_SEARCH_PHASE)
    {
        printf("Personality function, lookup phase\n");
        return _URC_HANDLER_FOUND;
    } else if (actions & _UA_CLEANUP_PHASE) {
        printf("Personality function, cleanup\n");

        // Calculate what the instruction pointer was just before the
        // exception was thrown for this stack frame
        uintptr_t throw_ip = _Unwind_GetIP(context) - 1;

        // Get a pointer to the raw memory address of the LSDA
        LSDA_ptr raw_lsda = (LSDA_ptr) _Unwind_GetLanguageSpecificData(context);

        // Create an object to hide some part of the LSDA processing
        LSDA lsda(raw_lsda);

        // Go through every entry on the call site table
        int i = 0;
        for (const LSDA_CS *cs = lsda.next_call_site_entry(true);
            cs != NULL;
            cs = lsda.next_call_site_entry())
        {
            printf("Found a CS #%i:\n", i);
            printf("\tcs_start: %i\n", cs->start);
            printf("\tcs_len: %i\n", cs->len);
            printf("\tcs_lp: %i\n", cs->lp);
            printf("\tcs_action: %i\n", cs->action);
            i++;
        }

        // Go through each call site in this stack frame to check whether
        // the current exception can be handled here
        // for(const LSDA_CS *cs = lsda.next_call_site_entry(true);
        //         cs != NULL;
        //         cs = lsda.next_call_site_entry())
        // {
        //     // If there's no landing pad we can't handle this exception
        //     if (not cs->lp) continue;

        //     uintptr_t func_start = _Unwind_GetRegionStart(context);

        //     // Calculate the range of the instruction pointer valid for this
        //     // landing pad; if this LP can handle the current exception then
        //     // the IP for this stack frame must be in this range
        //     uintptr_t try_start = func_start + cs->start;
        //     uintptr_t try_end = func_start + cs->start + cs->len;

        //     // Check if this is the correct LP for the current try block
        //     if (throw_ip < try_start) continue;
        //     if (throw_ip > try_end) continue;

        //     // Get the offset into the action table for this LP
        //     if (cs->action > 0)
        //     {
        //         // cs->action is the offset + 1; that way cs->action == 0
        //         // means there is no associated entry in the action table
        //         const size_t action_offset = cs->action - 1;
        //         const LSDA_ptr action = lsda.action_tbl_start + action_offset;

        //         // For a landing pad with a catch the action table will
        //         // hold an index to a list of types
        //         int type_index = action[0];

        //         // const void* catch_type_info = lsda.types_table_start;
        //         // char* catch_type_info_char = (char*) catch_type_info;
        //         // catch_type_info_char -= 4 * type_index;
        //         // const void* catch_type_info_void = (const void*) catch_type_info_char;
        //         //const std::type_info *catch_ti = (const std::type_info *) catch_type_info_void;
        //         const void* catch_type_info = lsda.types_table_start[ -1 * type_index ];
        //         const std::type_info *catch_ti = (const std::type_info *) catch_type_info;
        //         printf("%s\n", catch_ti->name());
        //     }

        //     // We found a landing pad for this exception; resume execution
        //     int r0 = __builtin_eh_return_data_regno(0);
        //     int r1 = __builtin_eh_return_data_regno(1);

        //     _Unwind_SetGR(context, r0, (uintptr_t)(unwind_exception));

        //     // Note the following code hardcodes the exception type;
        //     // we'll fix that later on
        //     _Unwind_SetGR(context, r1, (uintptr_t)(1));

        //     _Unwind_SetIP(context, func_start + cs->lp);
        //     break;
        // }

        return _URC_INSTALL_CONTEXT;
    } else {
        printf("Personality function, error\n");
        return _URC_FATAL_PHASE1_ERROR;
    }
}

}
