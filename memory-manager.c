#include "memory-manager.h"


// GLOBALS 
char 		library; 				// Container for super_page_tables for each thread
mem_book* 	book_keeper;			// An array of mem_books.  Keeps records of memory
char* 		memory;					// main memory.  An array of char ptrs
FILE* 		swap_file;				// swap file. 

int 	MEMORY_SIZE = 2<<22;		// 8MB  (8388608 bytes)
int 	SWAP_SIZE	= 2<<23; 		// 16MB (16777216 bytes)
int 	PAGE_SIZE 	= 0;			// Dynamically populated in init(). ~4096 bytes
int 	PAGES_IN_MEMORY = 0;		// Dynamically populated in init(). 2048 for PS=4096
int 	PAGES_IN_SWAP 	= 0;		// Dynamically populated in init(). 4096 for PS=4096

int 	STACK_SIZE 		= 128; 		// Size of a thread's stack
int 	MAX_THREADS 	= 0;		// Dynamically populated in init(). 
int 	initialized = 0;			// Boolean to check if mem-manger is init'ed


void initMemoryManager(){


	int i;
	PAGE_SIZE 		= sysconf(_SC_PAGE_SIZE);
	PAGES_IN_MEMORY = (MEMORY_SIZE / PAGE_SIZE);
	PAGES_IN_SWAP 	= (SWAP_SIZE / PAGE_SIZE);

	MAX_THREADS = (PAGE_SIZE/ (STACK_SIZE + sizeof(ucontext_t)));




	printf("%d\n", STACK_SIZE+sizeof(ucontext_t));
	printf("%d\n", MAX_THREADS);

	/****************************************************************************
	*			INIT MEMORY
	*
	****************************************************************************/	
	
	/* memory is an array of char ptrs. Each char* will point to a page */
	char* memory[PAGES_IN_MEMORY];

	/* Populating memory array.  Obtain pointers using memalign. */
	for(i=0; i<PAGES_IN_MEMORY; i++){
		// The memalign function allocates a block of size bytes whose address
		// is a multiple of boundary. The boundary must be a power of two! 
		// The function memalign works by allocating a somewhat larger block, 
		// and then returning an address within the block that is on the specified 
		// boundary.
		memory[i] = (char*) memalign(PAGE_SIZE, PAGE_SIZE);
	}


	/****************************************************************************
	*			INIT BOOK_KEEPER
	*
	****************************************************************************/	
	mem_book* book_keeper[PAGES_IN_MEMORY]; 




	/****************************************************************************
	*			INIT LIBRARY
	*
	****************************************************************************/	
	// library




	// printf("%d\n", sizeof(mem_book));
	// printf("%d\n", sizeof(book_keeper));






	initialized = 1;
}








void* scheduler_malloc(int size, int TID){return 0;}


void* myallocate(int size, char* FILE, int LINE){

	if(initialized == 0)
		initMemoryManager();


	return 0;
}


void* mydellocate(void* ptr){return 0;}










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


    initMemoryManager();
    printf("PageSize: %i\n",PAGE_SIZE);




    int entry = buildMemEntry(1, 0, 0, 28347);
    _printMemEntry(entry);





	/*
    // int addr = 0xFFFF8123;
    // int addr2 = 0xAAFF8123;
    User's Virtual Address:   1111 1111 1111 1111 1000 0001 0010 0011
                            = 0xFFFF8123

					0xAAFF8123 = 1010 1010 1111 1111 1000 0001 0010 0011	


    valid is 1 
    isfree is 1 
    right_dep is 1 
    request_size is 8356131 (right most 23)
    */
    return 0;

}

