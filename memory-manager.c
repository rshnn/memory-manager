#include "memory-manager.h"


// GLOBALS 
PTEntry* super_page_array[32]; 		// page table of page tables 
char 	memory[2 << 22];          					// main memory. char is 1 bytes.  Need 8MM 
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


void _printMemEntry(memEntry* x){

	printf(ANSI_COLOR_MAGENTA "v: %i | f: %i | r: %i | size: %i\n" ANSI_COLOR_RESET,
				 x->valid, x->isfree, x->right_dependent, x->request_size);

}



int main(){

	intializeSwapSpace();

    int addr = 0xFFFF8123;

    printf("%i\t%i\t%i\t%i\n", getValidBit(addr), getIsFreeBit(addr), getRightDepBit(addr), getRequestSize(addr));


	//_printMemEntry(&testing);
}

