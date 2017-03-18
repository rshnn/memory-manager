/****************************************************************************
*
* memory-manager.c
*
****************************************************************************/
#include "memory-manager.h"


char** 			memory;				// main memory.  An array of char ptrs
FILE* 			swap_file;			// swap file. 
MemBook* 		book_keeper;		// An array of MemBooks.  Keeps records of memory
SuperPTArray*	SPTA_library;		// Array of SPA's. One for each thread (index = TID)
ThrInfo** 		thread_list;		// Array of ThrInfo ptrs for all threads 
SwapUnit* 		swap_bank; 			// Array of SwapUnit structs.  Book keeping for swap file

int 	MEMORY_SIZE 	= 2<<22;	// 8MB  (8388608 bytes)
int 	SWAP_SIZE		= 2<<23; 	// 16MB (16777216 bytes)
int 	PAGE_SIZE 		= 0;		// Dynamically populated in init(). ~4096 bytes
int 	PAGES_IN_MEMORY = 0;		// Dynamically populated in init(). 2048 for PS=4096
int 	PAGES_IN_SWAP 	= 0;		// Dynamically populated in init(). 4096 for PS=4096
int 	STACK_SIZE 		= 128; 		// Size of a thread's stack TODO: CHANGE THIS IN MYPTHREAD
int 	MAX_THREADS 	= 0;		// Dynamically populated in init(). 8 for PS=4096
int 	VALID_PAGES_MEM	= 0;

int 	initialized 	= 0;		// Boolean to check if mem-manger is init'ed
int 	swap_count 		= 0;		// Number of pages in swap file occupied 

/***************************************************************************
****************************************************************************
*							HELPER FUNCTIONS
****************************************************************************
****************************************************************************/

/**
*	Helper Function
*		Builds an integer that represents a 32-bit memEntry	
* 		Usage:  int entry = initMemEntry(1,0,0,35010);
*/
int initMemEntry(int valid, int isfree, int left_dep, int request_size){
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
*		Returns 0 on success
*/
int buildThrInfo(int tid){

	int i;

	ThrInfo* temp 		= (ThrInfo*)malloc(sizeof(ThrInfo));
	temp->TID 			= tid;
	temp->num_blocks 	= 0;
	temp->num_pages 	= 0;
	temp->SPTArray 		= &(SPTA_library[tid]);			// Cannot make more than MAX_THREADS.  Will break here

	/* Array of ptblock pointers.  They point to nothing initially */
	temp->blocks 		= (PTBlock**)malloc(32*sizeof(PTBlock*));
	for(i=0; i<32; i++){
		temp->blocks[i] = NULL;						// Problem? 
	}

	/* Write to global thread_list structure */
	thread_list[tid] 	= temp;

	return 0;
}

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
	MAX_THREADS 	= (PAGE_SIZE/ (sizeof(ucontext_t)+4));			// 4 for mementries
	VALID_PAGES_MEM = PAGES_IN_MEMORY - 2; 			// Last 2 reserved for scheduler stuff 

	if(SHOW_PRINTS){	
		printf(ANSI_COLOR_CYAN"SYSTEM INFO:\n\tPAGE_SIZE:\t\t\t%i\n"ANSI_COLOR_RESET,\
			PAGE_SIZE);
		printf(ANSI_COLOR_CYAN"\tMEMORY_SIZE:\t\t\t%d\n"ANSI_COLOR_RESET, MEMORY_SIZE);
		printf(ANSI_COLOR_CYAN"\tPAGES_IN_MEMORY:\t\t%d\n"ANSI_COLOR_RESET, PAGES_IN_MEMORY);
		printf(ANSI_COLOR_CYAN"\tVALID_IN_MEMORY:\t\t%d\n"ANSI_COLOR_RESET, VALID_PAGES_MEM);
		printf(ANSI_COLOR_CYAN"\tSWAP_SIZE:\t\t\t%d\n"ANSI_COLOR_RESET, SWAP_SIZE);
		printf(ANSI_COLOR_CYAN"\tPAGES_IN_SWAP:\t\t\t%d\n"ANSI_COLOR_RESET, PAGES_IN_SWAP);
		printf(ANSI_COLOR_CYAN"\tsizeof(ucontext+stack):\t\t%d\n"ANSI_COLOR_RESET, \
			STACK_SIZE+sizeof(ucontext_t));
		printf(ANSI_COLOR_CYAN"\tMAX_THREADS:\t\t\t%d\n"ANSI_COLOR_RESET, MAX_THREADS);
		printf(ANSI_COLOR_CYAN"\t------------------------------------\n"ANSI_COLOR_RESET);
	}

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
		// (int)*(memory[i]) = initMemEntry(1, 1, 0, PAGE_SIZE); 		// ???????
	}



	/****************************************************************************
	*			INIT BOOK_KEEPER
	*
	****************************************************************************/	
	/* Each page in memory gets a MemBook to track who is currently resident */
	book_keeper = (MemBook*) malloc(VALID_PAGES_MEM * sizeof(MemBook));
	for(i=0; i<PAGES_IN_MEMORY; i++){
		book_keeper[i].isfree 				= 1;
		book_keeper[i].TID 					= -2;	// Made it -2 in case -1 causes problems		
		book_keeper[i].thread_page_number 	= -2;	// since we used -1 for a completed thread
		book_keeper[i].entry 				= NULL;  // pointer to PTE of the resident page 
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
			SPTA_library[i].array[j] 		= 0;
			SPTA_library[i].saturated[j] 	= 0;

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
		// thread_list[i] = NULL;
		buildThrInfo(i);
	}


	/****************************************************************************
	*			INIT SWAP_SPACE
	*
	****************************************************************************/	


	swap_file = fopen("swagmaster.swp", "w");
	// fseek(swap_file, 16777217, SEEK_SET);
	
	printf("tell before: %ld\n", ftell(swap_file));
	ftruncate(fileno(swap_file), 16777216);
	// write(fileno(swap_file), "thing", 5);
	printf("tell after: %ld\n", ftell(swap_file));


	// fputc('c', swap_file);
	close(fileno(swap_file));





	// lseek(fileno(swap_file), 16777216, SEEK_SET);
	// rewind(swap_file);



	/****************************************************************************
	*			INIT SWAP_BANK
	*
	****************************************************************************/	
	swap_bank = (SwapUnit*)malloc(PAGES_IN_SWAP* sizeof(SwapUnit));


	initialized = 1;
}





/**
*	Helper Function
*		Adds a new empty PTBlock to ThrInfo's block linked list.
* 		Input:  ThrInfo of owning thread
* 		Returns 0 on success 
*/
int addPTBlock(ThrInfo* thread){

	if(thread->num_blocks == 32){
		return -1;
	}

	int i;
	PTBlock* block 	= (PTBlock*)malloc(sizeof(PTBlock));
	block->TID 		= thread->TID;

	block->blockID 	= thread->num_blocks;

	/* Init all ptentries */
	for(i=0; i<128; i++){
		block->ptentries[i] = makePTEntry(0,0,0,0,0,PAGE_SIZE-32, 2048, 0);
	}

	thread->blocks[block->blockID] = block;	 		// BlockID is 0 indexed  If breaks here.  Null pointer issues?
	thread->num_blocks++; 							// Not 0 indexed 


	thread->SPTArray->array[block->blockID] 	= 1;
	thread->SPTArray->saturated[block->blockID] = 0;

	return 0;
}


PTBlock* nextAvailableBlock(ThrInfo* thread, int startwith){

	int i;
	int block_index = -1;
	SuperPTArray* sptarray = thread->SPTArray;

	for(i=startwith+1; i<32; i++){
		if(sptarray->array[i] == 1 && sptarray->saturated[i] == 0){
			// This block is initialized and unsaturated
			block_index = i;
			break;
		}
	}
	if(block_index == -1){
		// Need to add a new block 
		int success = addPTBlock(thread);
		if(success == -1){
			return NULL;
		}

		block_index = thread->num_blocks-1; 		// Grabs index of newest 
		// addPTBlock() will update the sptarray
	}
	return thread->blocks[block_index];

}









/****************************************************************************
****************************************************************************
*							LIBRARY FUNCTIONS
****************************************************************************
****************************************************************************/

void* scheduler_malloc(int size){return 0;}


void* myallocate(int size, char* FILE, int LINE, int tid){

	int i;
	int first_allocate = 0;
	
	if(initialized == 0)
		initMemoryManager();

	/*********************************************************************
	(0) Fetch ThrInfo for this thread from thread_list

	(0.5) If request > PAGESIZE, do stuff with giving back multiple pages
		Follow similar pattern to below, but check for enough contiguous pages

	(1) Locate PTBlock with available space
		Check SuperPTA linearly for a 1.
			Check saturated at that index to see if its full.
			If not, save that index=A.

	(2) Locate PTEntry with enough space 
		Go to Ath block (stored as array in ThrInfo)
			Iterate through PTEntries (block->ptentries[0 - 127])
			Check ptentries[i]->largest_available
			If request fits, save that index, i=B
		We need ThrInfo->TID's Page B

	(3)	Get my page into some spot in memory 

			Do i have an assigned spot in memory yet? (mem_pag!= 2048)
				Yes: Am i resident in this assigned spot? (book_keeper)
					Yes: Continue;
					No:  Move that guy into swap_bank 	(check his PTE)	


				No: Look for a free spot in memory. (book_keeper) (mem_page == 2048)
					
					If all full, look to evict.  	
						Is there enough swap_space?
							Failure return NULL.  Swap full
						Find the first MemBook that isnt mine
							get it out of here!
							My spot.  (save index to PTE->mem_page_number)

		
		Continue on to look for memEntry 
			// make a function call









		Is this page in memory rn?  Put it in if not.
		Go to book_keeper.  Check if [TID,page B] is currently loaded in memory
			book_keeper[B]->TID = mine?
			if not, put mine in. Save the other one in swap_file somewhere
	
	(4) Generate memEntry for this request
		Iterate through the page via memEntries.
		Find the one where request fits.  
		Do the memEntry things. 

	(5) Collect Pointer and return to thread
		Location of memEntry+ sizeof(memEntry)

		
	**********************************************************************/


	/********************************************/
	/* (0)  Fetch ThrInfo */ 
	ThrInfo* thread = thread_list[tid];
	if(thread->num_pages == 0)					
		first_allocate = 1;

	/********************************************/
	/* (1)  Locate PTBlock with available space */

	PTBlock* myblock 			= NULL; 		// Block with free space in it
	int 	block_index			= -1;
		// Case 1: First malloc by this thread.  Generate new PTBlock.  Update SupPTA
	if(first_allocate){
		addPTBlock(thread);
		myblock = thread->blocks[0];
		block_index = 0;
	}else{
		// Case 2: Regular case. 
		myblock = nextAvailableBlock(thread, 0);
		if(myblock == NULL){
			printf(ANSI_COLOR_RED"Cannot allocate anymore blocks to %i.\n"ANSI_COLOR_RESET, tid);
			return NULL;
		}		

	}
	// Ive got the block with available space now (myblock)			
	// TODO:  Havent done anything about requests > PAGESIZE yet



	/********************************************/
	/* (2) Locate PTEntry with enough space */

	int mypagenumber = -1;
	while(mypagenumber == -1){

		for(i=0; i<128; i++){
			PTEntry temp = myblock->ptentries[i];
			int available_size = temp.largest_available;
			
			// Does request fit in here?  (sizeof(int) = sizeof(memEntry))
			if(available_size > (size + sizeof(int))){
				mypagenumber = i;
				break;
			}
		}
		
		if(mypagenumber == -1){
			// myblock doesn't have a page with enough space
			myblock = nextAvailableBlock(thread, myblock->blockID);
		}
	}
	// I've got the pagenumber that this thread can use now (mypagenumber)
	PTEntry myPTE = thread->blocks[myblock->blockID]->ptentries[mypagenumber];

	if(swap_count >= 4095){
		printf(ANSI_COLOR_RED"" ANSI_COLOR_RESET);
		return NULL;
	}

	int myrealpagenumber = mypagenumber+(myblock->blockID)*128;


	/********************************************/
	/* (3)	Get my page into some spot in memory  */
		// Do i have an assigned spot in memory yet? (mem_page== 2048)
		// 	Yes: Am i resident in this assigned spot? (book_keeper)
		// 		Yes: Continue;
		// 		No:  Move that guy into swap_bank 	(check his PTE)	


		// 	No: Look for a free spot in memory. (book_keeper)
				
		// 		If all full, look to evict.  	
		// 			Is there enough swap_space?
		// 				Failure return NULL.  Swap full
		// 			Find the first MemBook that isnt mine
		// 				get it out of here!
		// 				My spot.  (save index to PTE->mem_page_number)


	char* ptr_startofpage = NULL;

	int myspotinmem = myPTE.mem_page_number;

	if(!(book_keeper[myspotinmem].TID == thread->TID)){

		// Write back guy in my spot
		PTEntry* guy = book_keeper[myspotinmem].entry;
		int guy_offset = guy->swap_page_number;

		rewind(swap_file);
		lseek(fileno(swap_file), guy_offset*PAGE_SIZE, SEEK_SET);
		write(fileno(swap_file), memory[myspotinmem], PAGE_SIZE);
		rewind(swap_file);

		// Write my swap file in 
		// First run:  dont have swap init'ed 

		// 	if swap_p_n==0, not in swap
	}
	// I am resident. 


	/* LEFT OFF HERE */







	/********************************************/
	/* (4) Generate memEntry for this request */
		// Iterate through the page via memEntries.
		// Find the one where request fits.  
		// Do the memEntry things. 
	


	// _printMemEntry(headME);

	/*INCOMPLETE.BRB*/



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

    // fclose(swap_file);

    initMemoryManager();

    // buildThrInfo(4);
    // _printThrInfo(thread_list[4]);

    // int entry = initMemEntry(1, 0, 0, 28347, 0);
    // _printMemEntry(entry);

    return 0;

}

