#include "memory-manager.h"


// GLOBALS 
PTEntry* 	super_page_array[32]; 	// page table of page tables
mem_book* 	book_keeper[4096];		// TODO:  Make this dyanmic.  Replace 4096
char* 	memory;	//[2 << 22];          		// main memory. char is 1 bytes.  Need 8MB 
FILE* 	swap_file;

int 	PAGE_SIZE 	= 0;			// Dynamically populated in init() 
int 	initialized = 0;			// Boolean to check if mem-manger is init'ed


void initMemoryManager(){

	PAGE_SIZE = sysconf(_SC_PAGE_SIZE);


	/* Need to allocate memory here with memalign */
	// But im confused.  The commented out for loop below this one is 
	// from that other group.  Need to research how memalign works. 
	// What exactly is that second input?  
	int s, find_front = 0;
	for(s = 0; s < (8000); s++){

		char* ptr = (char*)memalign(PAGE_SIZE, PAGE_SIZE);

		if(find_front = 0){
			memory = ptr;
			find_front = 1;
		}
	}

	// for(size=0;size<8000;size+=1){
	// 	char  *p=memalign(pagesize, 1024);
	// 	if(c==0){
	// 		physical=p;
	// 	}
	// 	c++;
	// }

	initialized = 1;
}








void* scheduler_malloc(int size, int TID){}


void* myallocate(int size, char* FILE, int LINE){

	if(initialized == 0)
		initMemoryManager();
}


void* mydellocate(void* ptr){}










/**
*	Helper Function
*		Builds an integer that represents a 32-bit memEntry	
* 		Usage:  int entry = buildMemEntry(1,0,0,35010);
*/
int buildMemEntry(int valid, int isfree, int left_dep, int request_size){
	int entry = 0;

	if (valid)
		entry += 0x80000000;
	if (isfree)
		entry += 0x40000000;
	if (left_dep)
		entry += 0x20000000;

	entry += request_size;

	return entry;
}


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

    printf(ANSI_COLOR_MAGENTA"ME_header: \t0x%x\t[v: %i | f: %i | rdep: %i | req_size: %i]\n"\
    	ANSI_COLOR_RESET, header, getValidBitME(header), getIsFreeBitME(header), \
    	getRightDepBitME(header), getRequestSizeME(header));

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

    initMemoryManager();
    printf("%i\n",PAGE_SIZE);




    int entry = buildMemEntry(1, 0, 0, 28347);
    _printMemEntry(entry);





	/*
    User's Virtual Address:   1111 1111 1111 1111 1000 0001 0010 0011
                            = 0xFFFF8123

					0xAAFF8123 = 1010 1010 1111 1111 1000 0001 0010 0011	


    valid is 1 
    isfree is 1 
    right_dep is 1 
    request_size is 8356131 (right most 23)
    */

}

