#ifndef MEMORY_MANAGER_H

#define MEMORY_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ucontext.h> 
#include <errno.h>
#include <sys/types.h>

#define SUPRESS_PRINTS 		1

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"




// #define malloc(x) mymalloc(x, __FILE__, __LINE__);
// #define free(x) myfree(x, __FILE__, __LINE__);


// virtual_addr 
// +---------8-------+--------12---------+--------12--------+
// |        TID      |     Page Number   |      Offset      |
// |                 |                   |                  |
// +-----------------+-------------------+------------------+
//
// 8 TID           : 256 SPT Entries 
// 12 PN           : 4096 Total pages 
// 12 Offset       : 4096 Size of Page 




//  OLD
// +---------5-------+--------12---------+------3------+----12----+
// |   SPT Index     |     Page Number   |   GARBAGE   | Offset   |
// |                 |                   |             |          |
// +-----------------+-------------------+-------------+----------+
//
// 5 SPT Index     : 32 SPT Entries 
// 12 PN           : 4096 Total pages 
// 12 Offset       : 4096 Size of Page 



/************************************************************************************************************
*
*    MEMORY-MANAGER DATA STRUCTURES
*
************************************************************************************************************/
    
/* To Remove.  Obsolete */
typedef struct memEntry_{

    unsigned int    valid: 1;
    unsigned int    isfree:1;
    unsigned int    right_dependent:1; 
    unsigned int    UNUSED:6;
    unsigned int    request_size:23;        // Max size is 8388608 (8MB)

}memEntry;


typedef struct PTEntry_{

    unsigned int    used:1;                 // Is the page currently used 
    unsigned int    resident:1;             // Is the page resident in memory 
    unsigned int    left_dependent:1;       // Do we need to load the next page
    unsigned int    right_dependent:1; 
    unsigned int    dirty:1;                // Indicates if the page has been written to (i.e needs to be written back to memory when evicted)
    unsigned int    UNUSED:3;
    unsigned int    largest_available:12;   // Size of largest fragment available inside the page
    unsigned int    page_number:12;         // Offset of page in memory (if it is loaded)


}PTEntry;




/************************************************************************************************************
*
*    BIT FIELD EXTRACTORS 
*
************************************************************************************************************/

/* memEntry is 32 bits */
/* REMOVE.  Obsolete. */
#define makeMemEntry(valid, isfree, right_dependent, request_size) \
    (struct memEntry_){ valid, isfree, right_dependent, 0, request_size}

/* PTEntry is 32 bits */
#define makePTEntry(used, resident, left_dependent, right_dependent, \
        dirty, largest_available, page_number) (struct PTEntry_){ \
        used, resident, left_dependent, right_dependent, dirty, 0, \
        largest_available, page_number}

//for virtual_addresses
// A virtual address has a three-part structure as follows:
// +---------8-------+--------12---------+--------12--------+
// |        TID      |     Page Number   |      Offset      |
// |                 |                   |                  |
// +-----------------+-------------------+------------------+
//
// 8 TID           : 256 SPT Entries 
// 12 PN           : 4096 Total pages 
// 12 Offset       : 4096 Size of Page 
// #define get
// #define 



//for memEntry
// +------1-------+-------1------+------1------+----6----+------23-----------+
// |     valid    |     isfree   |   right_dep | GARBAGE |  request_size     |
// |              |              |             |         |                   |
// +--------------+--------------+-------------+---------+-------------------+
//
// 23 request_size  : max request size is 8388608 (8MB) 

#define getValidBitME(header) ((header & 0x80000000)>>31)
#define getIsFreeBitME(header) ((header & 0x40000000)>>30)
#define getRightDepBitME(header) ((header & 0x20000000)>>29)
#define getRequestSizeME(header) (header & 0x007FFFFF)


//for page table entry 
/* NOTE: UNTESTED SOME1 PLS TEST THIS */
#define getUsedBitPT(entry) ((entry & 0x80000000)>>31)
#define getResidentBitPT(entry) ((entry & 0x40000000)>>30)
#define getLeftDependentBitPT(entry) ((entry & 0x20000000)>>29)
#define getRightDependentBitPT(entry) ((entry & 0x10000000)>>28)
#define getDirtyBitPT(entry) ((entry & 0x08000000)>>27)
#define getUnusedBitPT(entry) ((entry & 0x07000000)>>24) //unused
#define getLargestAvailable_BitPT(entry) ((entry & 0x00000088))
#define getPageNumberPT(entry) ((entry & 0x000000C))

/************************************************************************************************************
*
*    MEMORY-MANAGER FUNCTION LIBRARY
*
************************************************************************************************************/


void* scheduler_malloc(int size, int TID);
void* myallocate(int size, char* FILE, int LINE);
void* mydellocate(void* ptr);


#endif
