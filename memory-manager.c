#include "memory-manager.h"


// GLOBALS 
page_table_entry* super_page_array[32]; 		// page table of page tables 
char 	memory[2 << 22];          					// main memory. char is 1 bytes.  Need 8MM 
FILE* 	swap_file;


void* scheduler_malloc(int size, int TID){}
void* myallocate(int size, char* FILE, int LINE){}
void* mydellocate(void* ptr){}


void intializeSwapSpace(){

	swap_file = fopen("swagmaster.swp", "w+");

	lseek(fileno(swap_file), 1<24, SEEK_SET);
	rewind(swap_file);

}



int main(){

	intializeSwapSpace();

}

