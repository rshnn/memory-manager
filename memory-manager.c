#include "memory-manager.h"


// GLOBALS 
PTEntry* super_page_array[32]; 		// page table of page tables 
char 	memory[2 << 22];          	// main memory. char is 1 bytes.  Need 8MM 
FILE* 	swap_file;


void* scheduler_malloc(int size, int TID){}


void* myallocate(int size, char* FILE, int LINE){





	memEntry new = makeMemEntry(1, 0, 0, size);



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

    printf(ANSI_COLOR_MAGENTA"addr: %x:\t[v: %i | f: %i | rdep: %i | req_size: %i]\n"\
    	ANSI_COLOR_RESET, header, getValidBitME(header), getIsFreeBitME(header), \
    	getRightDepBitME(header), getRequestSizeME(header));

}




int main(){

	intializeSwapSpace();

    int addr = 0xFFFF8123;


	/*
    User's Virtual Address:   1111 1111 1111 1111 1000 0001 0010 0011
                            = 0xFFFF8123

    valid is 1 
    isfree is 1 
    right_dep is 1 
    request_size is 8356131 (right most 23)
    */



	_printMemEntry(addr);
}

