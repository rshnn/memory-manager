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



/************************************************************************************************************
*
*    MEMORY-MANAGER DATA STRUCTURES
*
************************************************************************************************************/
    

typedef struct memEntry_{

    unsigned int    valid: 1;
    unsigned int    isfree:1;
    unsigned int    right_dependent:1; 
    unsigned int    UNUSED:6;
    unsigned int    request_size:23;

}memEntry;


typedef struct page_table_entry_{

    unsigned int    used:1;                 // Is the page currently used 
    unsigned int    resident:1;             // Is the page resident in memory 
    unsigned int    left_dependent:1;       // Do we need to load the next page
    unsigned int    right_dependent:1; 
    unsigned int    dirty:1;                // Indicates if the page has been written to (i.e needs to be writen back to memory when evicted)
    unsigned int    UNUSED:3;
    unsigned int    largest_available:12;   // Size of largest fragment availble inside the page
    unsigned int    page_number:12;         // Offset of page in memory (if it is loaded)


}page_table_entry;




/************************************************************************************************************
*
*    BIT FIELD EXTRACTORS 
*
************************************************************************************************************/

/* memEntry is 32 bits */
#define makeMemEntry(valid, isfree, right_dependent, request_size) \
    (struct memEntry_){ valid, isfree, right_dependent, 0, \
        request_size & 0x7fffff}



/************************************************************************************************************
*
*    MEMORY-MANAGER FUNCTION LIBRARY
*
************************************************************************************************************/


void* scheduler_malloc(int size, int TID);
void* myallocate(int size, char* FILE, int LINE);
void* mydellocate(void* ptr);



#endif
