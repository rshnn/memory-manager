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

 
 /* Converts an integer to binary (binary represented as integer) */
int binary_conversion(int num){
    if (num == 0)
        return 0;
    else
        return (num % 2) + 10 * binary_conversion(num / 2);
}





int main(){

	// intializeSwapSpace();
	int test = 3824;

	memEntry testing = makeMemEntry(0, 1, 1, test);

	printf("%i in binary is:\t\t%i\n", test, binary_conversion(test));
	printf("Is request size correct? \t%i\n", binary_conversion(testing.request_size));

}

