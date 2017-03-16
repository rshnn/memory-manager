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
#define getTIDVA(va) ((va & 0xFF000000)>> 24)
#define getPageNumberVA(va) ((va & 0x00FFF000) >> 12)
#define getOffsetVA(va) (va & 0x00000FFF)



/**
*	Private Helper Function
* 		Prints a user virtual address.
*		Input:  32-bit addr as integer. 	
*			addr{	
*					8:	TID,
*					12:	page number
*					12:	offset
*			}
*
*/
void _printVirtualAddr(int addr){
    printf(ANSI_COLOR_MAGENTA "virt_addr:\t%x\t[TID: %i\tPN: %i\toff: %i]\n" ANSI_COLOR_RESET, \
    	addr, getTIDVA(addr), getPageNumberVA(addr), getOffsetVA(addr));

}


/* To Remove.  Obsolete */
typedef struct memEntry_{

    unsigned int    valid: 1;
    unsigned int    isfree:1;
    unsigned int    right_dependent:1; 
    unsigned int    UNUSED:6;
    unsigned int    request_size:23;        // Max size is 8388608 (8MB)

}memEntry;
