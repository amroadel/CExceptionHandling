# CExceptionHandling
A simplistic exception handling framework to mimic the one found in gcc/libsupc++  
___
The repositary is based upon the code explained in Nico Brailovsky's blog:  
https://monoinfinito.wordpress.com/series/exception-handling-in-c/
and the original source code can be found in his repositary with our changes in a separate pull request:  
https://github.com/nicolasbrailo/cpp_exception_handling_abi

## Table of content
1. Research steps and issues found
  - ULEB128 encoding
  - Void* size
  - Table type entries encoding
2. LSDA table break down
  - Searching the LSDA for a handler
  - TTBase_encoding break down

## Research steps and issues found
The research in implementing a simplified ABI that can handle c++ exceptions firstly began by reviewing Brailovsky's blog. He tried to discover the minimum set of functions needed to have a working ABI by linking a throwing cpp file to a normal c file (because c doesn't have exception handling capabilities). He compiled the files on a x86 32 bit machine using gcc and added the missing functions required by the compiler one by one in a separate file of his making. The file contained functions originaly declared in libsupc++ (c++ support library) and were intended to solve the linking error of missing references as gcc dosn't have access to libsupc++. While it did solve the problem on the same environment, when trying on a x86 64 bit machine, segmentation fault error occures for various reasons starting from v08 that will be disscussed below.  
PS. there are little to no documentation about the LSDA area so the majority of this is discovered mainly through disassembly and tracing the gcc libsupc++ libraries
### ULEB128 encoding
As mentioned in the blog (prior knowledge of the source code from the blog until v08 is required), the personality function iterates through the LSDA call site table (LSDACS) entries in search for a proper handler. the entries are encoded in ULEB128 formate (for god knows why) and one of the assumptions that sadly wasn't true is that the values will be small enough so that their encoded value will be the same as the original (check [ULEB128 encoding](https://en.wikipedia.org/wiki/LEB128) for more info about the encoding process). The values stored represent relative addresses for possible handlers and while the assumption holds true for this case on a 32 bit machine, the longer machine code in a 64 bit resulted in a higher value for the relative addresses ultimately exceeding 127 (which is the maximum value that has the same encoding as its original) and as result the value read directly from the call site table wasn't the ones we needed and we had to proberly decode them. To solve this issue, a simple decoding function 'read_uleb128' was implemented and called each time a uleb128 entry was accessed. The result is, we can now access all the entries (with the correct values) in the lsda table, but it's never too easy. We still had the segmentation fault error, and the reason this time was another architicture dependant issue.
### Void* size
After finding the correct handler from the call site table, the next step is accessing its related entry in the action table (and we can do this without a problem). The problem happens when we try jumping to the needed entry in the type table (basic understanding of the exception handling flow is required. An overview can be found later in the documen). Entries in type table are of type long and hold addresses to the needed data types (of type 'type_info') to check the thrown exception type. '.long' data type in x86 assembly (mostly anywhere actually) have a size of 4 bytes. The original source code iterated the table with the help of a simple void pointer and some pointer arthematics. ++1-ing a void pointer simply adds its size ('sizeof(void*)' which is 4 byte for 32 bit architicture) to the address contained in the pointer and this is how we access subsequent entries in the table, so far so good. Now when we go to a 64 bit machine, the size of the void* increases to 8 bytes but the entries are still spaced at 4 byte (because the .long size doesn't change with the architicture) and what happens is a troublsome out of bounds case. by modifying the mechanism of iteration (modifying the pointer size) we were able to access the correct address value. Now you would think that by jumping to this address we can have our required 'type_info', little did we know it wasn't that simple.
### Table type entries encoding
In one of the earlier version when we first started reading the lsda, we came across a '.byte' entry that we ignored assuming that it holds unnecessary information about the encoding of the type table. Well, this seamingly innocent single byte value should be masked with 3 different values to get the appropriate size, encoding, and relative address base. Now the size and encoding were not a problem (well we already solved the size issue and the size directives made it clear that this is a 4 byte value). The relative address base on the other hand was wrongfully ignored. It was assumed in the blog that the adress was an absolute address to a location in the assembly code. It turns out it's a pc-relative address (relative to the location of the current entry to be exact) after tracing the eh-personality of the libsupc++ library. In order to get the correct address, just add the value found in the type table to the address of the value in the table (this was implemented in 'get_ttype_entry'). And there you have it, v08 now finally works on an x86_64 machine. We were hoping that propagating the changes to v12 would solve everything, but '__cxa_throw' function had another say in the matter. We are still in the process of debugging it.

## LSDA table break down
In order to understand the exception handling process, a through understanding of the lsda area is a must. this section provides an insight on each field. The assembly code generated from the throw.cpp file in the blog is taken as an example  
Note the following abbriviations:  
- @LP: landing pad
- @TT: type table
- @CS: call site
- @AT: action table
- @AR: action record

| Section | Field Name | Size | Value(Example) | Description |
|-------------------|--------------------|-----------|-------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------|
| LSDA Header | @LPStart_encoding | .byte | 0xff | The encoding used in the next field (0xff means omitted) |
|  | @LPStart(optional) | (omitted) | (omitted) | The relative address base for entries (call sites) in the call site table (if removed, addresses will be relative to function start) |
|  | @TTBase_encoding | .byte | 0x9b | According to the mask applied, it can give other info about the stored entries (Mask in a separate table) |
|  | @TTBase | .uleb128 | .LLSDATT1-.LLSDATTD1 | type_table_start - call_site_table_start: offset to the type table from after the lsda header |
|  | @CSTable_encoding | .byte | 0x1 | The encoding used in the call site table records (entries). In this case it's uleb128 |
|  | @CSTable_size | .uleb128 | .LLSDACSE1-.LLSDACSB1 | call_site_table_end - call_site_table_start: call site table size. It's also the action table offset as it comes directly after cs table |
| Call Site Records | @CS | .uleb128 | .LEHB0-.LFB1 | call_site_start - function_start: call site relative address (relative to @LPStar) |
|  | @CS_size | .uleb128 | .LEHE0-.LEHB0 | call_site_end - call_site_start: call site size |
|  | @LP | .uleb128 | .L13-.LFB1 | landing_pad - @LPStart: relative address of the landing pad specific to this call site relative to @LPStart (or function start if omitted) |
|  | @AT_offset | .uleb128 | 0x1 | offset from action_table_start to the proper handler of this call site from the action table (previous 2 values will be 0 in case of no handler) |
| ... | ... | ... | ... |  |
| Action Table | @TT_offset | .byte | 0x1 | offset from type_table_start to the type handled by this landing pad |
|  | @AR_offset | .byte | 0x7d | self relative offset to the next entry. although the size directive is byte, it's actually a leb128 encoded value (-3 is most cases) |
| ... | ... | ... | ... |  |
|  | @TypeInfo_-2 | .long | DW.ref._ZTI9Exception-. | type table base is at the bottom and subsequent entries are accessed in reverse order by decrementing from the base |
| Type Table | @TypeInfo_-1 | .long | DW.ref._ZTI14Fake_Exception-. | pc relative address to the type info |
### Searching the LSDA for a handler
Iterating the LSDA happens as follows:
1. when an exception is thrown the '__cxa_throw' method initializes a chain of events that gets us to a point where the personality function is at the begining of the first call site in the call site table
2. if the value of '__Unwind_GetIP()' doesn't fall between @CS and @CS + @CS_size skip to the next call site
3. if the value falls in the call site range check the value of @LP for this call site record
4. if it's zero then there is no handler, skip to the next call site record
5. if it's non-zero check it's @AT_offset
6. if it's zero then there is no action (skip the call site)
7. if it's non zero then jump to action_table_start[(@AT_offset-1)] and check the value of @TT_offset in this action record
8. jump to type_tabel_start[-@TT_offset] and access the type info following the rules found in @TTBase_encoding
9. if the type info matches the one thrown then you have found the appropriate handler in @LP
10. if it doesn't then back to the action record check @AR_offset for the next action record relative offset, if it was zero then this is the end and you can't handle this exception with this landing pad (skip it)
### TTBase_encoding break down
The info stored in @TTBase_encoding can be obtained by applying different masks  

To get the size of the encoded value mask it with 0x07  
|Mask Result  |Size           |
|-------------|---------------|
|0x00         |sizeof(void*)  |
|0x02         |2 bytes        |
|0x03         |4 bytes        |
|0x04         |8 bytes        |

To get the encoding and data type mask it with 0x0f  
|Mask Result  |Type               |
|-------------|-------------------|
|0x00         |void*              |
|0x01         |uleb128            |
|0x09         |sleb128            |
|0x02         |unsigned 2 bytes   |
|0x03         |unsigned 4 bytes   |
|0x04         |unsigned 8 bytes   |
|0x0a         |signed 2 bytes     |
|0x0b         |signed 4 bytes     |
|0x0c         |signed 8 bytes     |

There is a special treatment if the value of @TTBase_encoding is 0x50 (if it's of reference aligned)  

To get the base of the relative address mask it with 0x70
|Mask Result  |Reference          |
|-------------|-------------------|
|0x00         |absolute ptr       |
|0x10         |pc relative        |
|0x50         |aligned*           |
|0x20         |text relative      |
|0x30         |data relative      |
|0x40         |function relative  |

*aligned is still unknown and for know is deemed unnecessary
