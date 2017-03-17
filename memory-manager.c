/****************************************************************************
*
* memory-manager.c
*
****************************************************************************/
#include "memory-manager.h"


char** 			memory;				// main memory.  An array of char ptrs
MemBook* 		book_keeper;		// An array of MemBooks.  Keeps records of memory
SuperPTArray*	SPTA_library;		// Array of SPA's. One for each thread (index = TID)
FILE* 			swap_file;			// swap file. 
ThrInfo** 		thread_list;		// Array of ThrInfo ptrs for all threads 


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
*			memory, book_keeper, SPTA_library, swap_file
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
	*			INIT SPTA_library
	*
	****************************************************************************/	
	/* Each thread gets a SuperPTArray */
	SPTA_library = (SuperPTArray*) malloc( MAX_THREADS * sizeof(SuperPTArray));

	/* Init all values to 0. */
	for(i=0; i<MAX_THREADS; i++){
		for(j=0; j<32; j++){
			SPTA_library[i].array[j] = 0;
		}
		SPTA_library[i].TID = i;
	}


	/****************************************************************************
	*			INIT THREAD_LIST
	*
	****************************************************************************/	
	/* One ThrInfo struct ptr per thread.  The structs is NOT allocated here. */
	thread_list = (ThrInfo**)malloc(MAX_THREADS * sizeof(ThrInfo*));
	/* Init all the pointers to NULL.  They will be populated by makeThrInfo() */
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
*		Builds a new empty ThrInfo struct.  
* 			Saves a pointer to respective index in thread_list
* 		Input:  TID of owning thread
*/
void buildThrInfo(int tid){

	ThrInfo* temp 	= (ThrInfo*)malloc(sizeof(ThrInfo));
	temp->TID 			= tid;
	temp->num_blocks 	= 0;
	temp->num_pages 	= 0;
	/* Write to global thread_list structure */
	thread_list[tid] 	= temp;
}


/**
*	Helper Function
*		Builds a new empty PTBlock.
* 		Input:  ThrInfo of owning thread
*/
PTBlock* buildPTBlock(ThrInfo* thread){

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

void* scheduler_malloc(int size){return 0;}


void* myallocate(int size, char* FILE, int LINE, int tid){

	if(initialized == 0)
		initMemoryManager();

	/*********************************************************************
	(0) Fetch ThrInfo for this thread from thread_list

	(1) Check for 1 in SPT, otherwise make a new entry
	
	(2) For each PTEntry, check if the largest available can meet the request 
			Otherwise, make a new page 

	(3) If request size is >4096 then just give it a new page (new PTEntry)

	(4) Once we've found the page with enough space...
		Look for the free memEntry header or append a new one to the end
		Put this into a helper function.  findNextLargestAvailable()

	(5) We can then load the page into memory if it isnt already

	(6) Collect the pointer to return to the user.
		memory[PAGENUM] + some offset + sizeof(its own memEntry)  	

	**********************************************************************/

	/* 0.  Find ThrInfo */ 
	ThrInfo* thread = thread_list[tid];
	if(thread->TID == -2){					// all initalized to -2 in init()
		buildThrInfo(tid);
		thread = thread_list[tid];
	}
	/*1.  Check for 1 in SPTA_library*/

	SuperPTArray mySPTArray = SPTA_library[tid];

	// Case1:  First malloc by this thread.  Generate new PTBlock.  Update SupPTA
	if(mySPTArray.array[0] == 0){

		PTBlock* newblock = buildPTBlock(thread);
	/*INCOMPLETE.BRB*/
	}


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
* 		Prints a ThrInfo struct
*/
void _printThrInfo(ThrInfo* thread){
	if(!SHOW_PRINTS)
		return;

	printf(ANSI_COLOR_MAGENTA "ThreadID: %i\tNumBlocks:%i\tNumPages:%i\n" ANSI_COLOR_RESET, \
		thread->TID, thread->num_blocks, thread->num_pages);
}


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


    // buildThrInfo(4);
    // _printThrInfo(thread_list[4]);

    int entry = buildMemEntry(1, 0, 0, 28347);
    _printMemEntry(entry);



    return 0;

}

