#include "memory-manager.h"


// GLOBALS 
PTEntry* super_page_array[32]; 		// page table of page tables 
char 	memory[2 << 22];          	// main memory. char is 1 bytes.  Need 8MM 
FILE* 	swap_file;


void* scheduler_malloc(int size, int TID){}


void* myallocate(int size, char* FILE, int LINE){




}


void* mydellocate(void* ptr){}



void intializeSwapSpace(){

	swap_file = fopen("swagmaster.swp", "w+");

	lseek(fileno(swap_file), 1<24, SEEK_SET);
	rewind(swap_file);

}


/**
*	Private Helper Function
* 		Prints a memEntry header.
*		Input:  32-bit header as integer. 	
*			header{	
*					1:	valid,
*					1:	isfree,
*					1:	right_dependent
*					6:	unused
*					23:	request size
*			}
*
*/
void _printMemEntry(int header){

    printf(ANSI_COLOR_MAGENTA"ME_header: \t%x\t[v: %i | f: %i | rdep: %i | req_size: %i]\n"\
    	ANSI_COLOR_RESET, header, getValidBitME(header), getIsFreeBitME(header), \
    	getRightDepBitME(header), getRequestSizeME(header));

}

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


/**
*	Private Helper Function
* 		Prints a page table entry.
*		Input:  32-bit entry as integer. 	
*			entry{	
*					1:	used,
*					1:	resident,
*					1:	left_dep,
*					1:	right_dep,
*					1:	dirty,
*					3:	unused
*					12:	largest_avail,
*					12: page_number
*			}
*
*/
void _printPageTableEntry(int entry){

	printf(ANSI_COLOR_MAGENTA"PTEntry: \t%x\t[u:%i |r:%i |ld:%i |rd:%i |d:%i |la:%i |pn: %i]\n" ANSI_COLOR_RESET, \
		entry, getUsedBitPT(entry), getResidentBitPT(entry), getLeftDependentBitPT(entry), \
		getRightDependentBitPT(entry), getDirtyBitPT(entry), getLargestAvailable_BitPT(entry), \
		getPageNumberPT(entry));

}




int main(){

	intializeSwapSpace();

    int addr = 0xFFFF8123;
    int addr2 = 0xAAFF8123;


	/*
    User's Virtual Address:   1111 1111 1111 1111 1000 0001 0010 0011
                            = 0xFFFF8123

					addr2 = 1010 1010 1111 1111 1000 0001 0010 0011	


    valid is 1 
    isfree is 1 
    right_dep is 1 
    request_size is 8356131 (right most 23)
    */


	_printVirtualAddr(addr);
	_printPageTableEntry(addr2);
	_printMemEntry(addr);
}

