/****************************************************************************
*
* memory-manager.c
*
****************************************************************************/
#include "memory-manager.h"


char** 			memory;				// main memory.  An array of char ptrs
MemBook* 		book_keeper;		// An array of MemBooks.  Keeps records of memory
SuperPTArray*	SPT_library;		// Array of SPA's. One for each thread (index = TID)
FILE* 			swap_file;			// swap file. 
ThrMemInfo** 	thread_list;		// Array of ThrMemInfo ptrs for all threads 


int 	MEMORY_SIZE 	= 2<<22;	// 8MB  (8388608 bytes)
int 	SWAP_SIZE		= 2<<23; 	// 16MB (16777216 bytes)
int 	PAGE_SIZE 		= 0;		// Dynamically populated in init(). ~4096 bytes
int 	PAGES_IN_MEMORY = 0;		// Dynamically populated in init(). 2048 for PS=4096
int 	PAGES_IN_SWAP 	= 0;		// Dynamically populated in init(). 4096 for PS=4096
int 	STACK_SIZE 		= 128; 		// Size of a thread's stack TODO: CHANGE THIS IN MYPTHREAD
int 	MAX_THREADS 	= 0;		// Dynamically populated in init(). 8 for PS=4096

int 	initialized 	= 0;		// Boolean to check if mem-manger is init'ed


/****************************************************************************
****************************************************************************
*							HELPER FUNCTIONS
****************************************************************************
****************************************************************************/

/**
*	Helper Function
*		Initializes all the structures needed to manage memory:
*			memory, book_keeper, SPT_library, swap_file
*/
void initMemoryManager(){


	int i,j;
	PAGE_SIZE 		= sysconf(_SC_PAGE_SIZE);
	PAGES_IN_MEMORY = (MEMORY_SIZE / PAGE_SIZE);
	PAGES_IN_SWAP 	= (SWAP_SIZE / PAGE_SIZE);
	MAX_THREADS 	= (PAGE_SIZE/ (STACK_SIZE + sizeof(ucontext_t)));

	// printf("Page Size:\t\t\t%i\n",PAGE_SIZE);
	// printf("Total size of ucontext+stack:\t%d\n", STACK_SIZE+sizeof(ucontext_t));
	// printf("Max Number of Threads:\t\t%d\n", MAX_THREADS);




	/****************************************************************************
	*			INIT MEMORY
	*
	****************************************************************************/	
	
	/* memory is an array of char ptrs. Each char* will point to a page */
	memory = (char **) malloc( PAGES_IN_MEMORY * sizeof(char*));

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
	/* Each page in memory gets a MemBook to track who is currently resident */
	book_keeper = (MemBook*) malloc(PAGES_IN_MEMORY * sizeof(MemBook));
	for(i=0; i<PAGES_IN_MEMORY; i++){
		book_keeper[i].isfree 				= 1;
		book_keeper[i].TID 					= -2;	// Made it -2 in case -1 causes problems		
		book_keeper[i].thread_page_number 	= -2;	// since we used -1 for a completed thread
	}


	/****************************************************************************
	*			INIT SuperPTLIBRARY
	*
	****************************************************************************/	
	/* Each thread gets a SuperPTArray */
	SPT_library = (SuperPTArray*) malloc( MAX_THREADS * sizeof(SuperPTArray));

	/* Init all values to 0. */
	for(i=0; i<MAX_THREADS; i++){
		for(j=0; j<32; j++){
			SPT_library[i].array[j] = 0;
		}
		SPT_library[i].TID = i;
	}


	/****************************************************************************
	*			INIT THREAD_LIST
	*
	****************************************************************************/	
	/* One ThrMemInfo struct ptr per thread.  The structs is NOT allocated here. */
	thread_list = (ThrMemInfo**)malloc(MAX_THREADS * sizeof(ThrMemInfo*));
	/* Init all the pointers to NULL.  They will be populated by makeThrMemInfo() */
	for(i=0; i<MAX_THREADS; i++){
		thread_list[i] = NULL;
	}


	/****************************************************************************
	*			INIT SWAP_SPACE
	*
	****************************************************************************/	
	swap_file = fopen("swagmaster.swp", "w+");
	lseek(fileno(swap_file), 1<24, SEEK_SET);
	rewind(swap_file);


	initialized = 1;
}


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


/**
*	Helper Function
*		Builds a new empty ThrMemInfo struct.  
* 			Saves a pointer to respective index in thread_list
* 		Input:  TID of owning thread
*/
void buildThrMemInfo(int tid){

	ThrMemInfo* temp 	= (ThrMemInfo*)malloc(sizeof(ThrMemInfo));
	temp->TID 			= tid;
	temp->num_blocks 	= 0;
	temp->num_pages 	= 0;
	thread_list[tid] 	= temp;
}


/**
*	Helper Function
*		Builds a new empty PTBlock.
* 		Input:  ThrMemInfo of owning thread
*/
PTBlock* buildPTBlock(ThrMemInfo* thread){

	PTBlock* block 	= (PTBlock*)malloc(sizeof(PTBlock));
	block->TID 		= thread->TID;

	block->blockID 	= thread->num_blocks;
	thread->num_blocks++;

	return block;
}









/****************************************************************************
****************************************************************************
*							LIBRARY FUNCTIONS
****************************************************************************
****************************************************************************/

void* scheduler_malloc(int size, int TID){return 0;}


void* myallocate(int size, char* FILE, int LINE){

	if(initialized == 0)
		initMemoryManager();


	return 0;
}


void* mydellocate(void* ptr){return 0;}









/****************************************************************************
****************************************************************************
*							DEBUGGING FUNCTIONS
****************************************************************************
****************************************************************************/


/**
*	Private Debugging Function
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
	if(!SHOW_PRINTS)
		return;
    printf(ANSI_COLOR_MAGENTA"ME_header: \t0x%x\t[v: %i | f: %i | rdep: %i | req_size: %i]\n"\
    	ANSI_COLOR_RESET, header, getValidBitME(header), getIsFreeBitME(header), \
    	getRightDepBitME(header), getRequestSizeME(header));

}




/**
*	Private Debugging Function
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
	if(!SHOW_PRINTS)
		return;
	printf(ANSI_COLOR_MAGENTA"PTEntry: \t%x\t[u:%i |r:%i |ld:%i |rd:%i |d:%i |la:%i |pn: %i]\n" ANSI_COLOR_RESET, \
		entry, getUsedBitPT(entry), getResidentBitPT(entry), getLeftDependentBitPT(entry), \
		getRightDependentBitPT(entry), getDirtyBitPT(entry), getLargestAvailable_BitPT(entry), \
		getPageNumberPT(entry));

}




int main(){


    initMemoryManager();



    int entry = buildMemEntry(1, 0, 0, 28347);
    _printMemEntry(entry);



    return 0;

}

