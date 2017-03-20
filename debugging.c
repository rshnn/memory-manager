/*  
	BUNCH OF OLD RETIRED DEBUGGING FUNCTIONS
		COPY PASTE INTO MY_PTHREAD_T.C AND RUN THROUGH MAIN. 
*/




/* 
	Tests the thread_unit library.  
	Stresses the thread_unit_list linked list structure  
		Retired: Fri 17 Feb 2017 10:43:58 AM EST
*/
void _debugging_thread_unit_lib(){
	

	// rshnn Tue 14 Feb 2017 12:20:09 PM EST
	printf(ANSI_COLOR_RED "\n\nRunning thread_unit_lib debug test...\n\n" ANSI_COLOR_RESET);

	int i = 0;
	my_pthread_t* 		pthread_arr[10];
	thread_unit* 		unit_arr[10];
	thread_unit_list* 	list =  thread_list_init();

	while(i <= 10){

		pthread_arr[i] = (my_pthread_t*)malloc(sizeof(my_pthread_t));
		pthread_arr[i]->threadID = i;		
		unit_arr[i] = thread_unit_init(pthread_arr[i]);
		// _print_thread_unit(unit_arr[i]);

		thread_list_enqueue(list, unit_arr[i]);

		i++;
	}

	printf("\nShould show ID's 0 to 10\n");
	_print_thread_list(list);


	/* Pops off head.  Threads 0 to 3 removed */
	thread_list_dequeue(list);
	thread_list_dequeue(list);
	thread_list_dequeue(list);
	thread_list_dequeue(list);

	/* Peek head.  Should not alter list. */
	printf("\nPeeking.  Should show thread 4:\n");
	_print_thread_unit(thread_list_peek(list));


	/* Add new thread to queue (ID = 99) */
	my_pthread_t* temp = (my_pthread_t*)malloc(sizeof(my_pthread_t));
	temp->threadID = 99;
	thread_unit* tempunit = thread_unit_init(temp);
	thread_list_enqueue(list, tempunit);


	/* List should have the following IDs:  4, 5, 6, 7, 8, 9, 10, 99 */
	printf("\nShould show the following IDs: 4, 5, 6, 7, 8, 9, 10, 99\n");
	_print_thread_list(list);


	/* Dequeue all thread_units off the list */
	for(i=0; i<=8; i++){
		thread_list_dequeue(list);
	}
	printf("\nShould show empty list: \n");
	_print_thread_list(list);



	/* Add  ID 99 and 5 back to queue */
	thread_list_enqueue(list, tempunit);
	thread_list_enqueue(list, unit_arr[5]);
	printf("\nShould show only thread 99: \n");
	_print_thread_list(list);
}




/* 
	Tests the pthread_create() function 
		Retired: Fri 17 Feb 2017 10:43:46 AM EST
*/
void _debugging_pthread_create(){


	printf(ANSI_COLOR_RED "\n\nRunning pthread_create() debug test...\n\n" ANSI_COLOR_RESET);
	
	/* Get main's ucontext */
	main_ucontext = (ucontext_t*)malloc(sizeof(ucontext_t));
	main_ucontext->uc_stack.ss_sp = malloc(PAGE_SIZE);
	main_ucontext->uc_stack.ss_size = PAGE_SIZE;
	getcontext(main_ucontext);
	makecontext(main_ucontext, (void*)_debugging_pthread_create, 1, NULL);


	scheduler_init();

	my_pthread_t* pthread_array[10];
	my_pthread_attr_t* 	useless_attr;
	int i;

	for(i=0; i<=10;i++){

		pthread_array[i] = (my_pthread_t*)malloc(sizeof(my_pthread_t));

		if(my_pthread_create(pthread_array[i], useless_attr, (void*)f1, (void*) i)){
			printf(ANSI_COLOR_GREEN "Successfully created pthread and enqueued.\n" ANSI_COLOR_RESET);
		}
	} 

	printf("\nPrinting thread list.  Should include pthreads 2 to 12.\n");
	_print_thread_list(scheduler->priority_array[0]);

		

	while(1);

	printf(ANSI_COLOR_RED "End pthread_create() debug test." ANSI_COLOR_RESET);
}



void _debugging_pthread_yield(){


	printf(ANSI_COLOR_RED "\n\nRunning pthread_yield() debug test...\n\n" ANSI_COLOR_RESET);
	
	int NUM_PTHREADS = 5;


	/* init scheduler...calls sig handler */
	// Move this into the first run of pthread_create() 
	scheduler_init();

	my_pthread_t pthread_array[NUM_PTHREADS];
	my_pthread_attr_t* 	useless_attr;
	int i;

	for(i=0; i<NUM_PTHREADS;i++){

		
		if(my_pthread_create(&pthread_array[i], useless_attr, (void*)f1, (void*) i)){
			printf(ANSI_COLOR_GREEN "Successfully created pthread and enqueued.\n" ANSI_COLOR_RESET);
		}
	} 

	printf("\nPrinting thread list.  Should include pthreads 2 to 6.\n");
	_print_thread_list(scheduler->priority_array[0]);



		

	while(1);	
	printf(ANSI_COLOR_RED "End pthread_yield() debug test." ANSI_COLOR_RESET);

}


void _debugging_pthread_join(){

	printf(ANSI_COLOR_RED "\n\nRunning pthread_join() debug test...\n\n" ANSI_COLOR_RESET);
	
	int NUM_PTHREADS = 5;


	// my_pthread_t pthread_array[NUM_PTHREADS];
	my_pthread_t* pthread_array = (my_pthread_t*)malloc(NUM_PTHREADS * sizeof(my_pthread_t));
	my_pthread_attr_t* useless_attr;

	int i;

	for(i=0; i<NUM_PTHREADS;i++){

		/* TID 2: Give last pthread functionptr to f2 (f2 terminates after a few seconds) */
		if(i == 0){
			if(my_pthread_create(&pthread_array[i], useless_attr, (void*)f2, (void*) i)){
				printf(ANSI_COLOR_GREEN "Successfully created f2 pthread and enqueued. TID %ld\n" 
					ANSI_COLOR_RESET, pthread_array[i].threadID);
			}
			continue;
		}
		
		/* TID 3:  f3 (joins TID2) */
		if(i == 1){
			if(my_pthread_create(&pthread_array[i], useless_attr, (void*)f3, (void*) &pthread_array[0])){
				printf(ANSI_COLOR_GREEN "Successfully created f3 pthread and enqueued. TID %ld\n" 
					ANSI_COLOR_RESET, pthread_array[i].threadID);
			}
			continue;
		}

		/* TID 4: f3 (joins TID2) */
		if(i == 2){
			if(my_pthread_create(&pthread_array[i], useless_attr, (void*)f3, (void*) &pthread_array[0])){
				printf(ANSI_COLOR_GREEN "Successfully created f3 pthread and enqueued. TID %ld\n" 
					ANSI_COLOR_RESET, pthread_array[i].threadID);
			}
			continue;
		}




		if(my_pthread_create(&pthread_array[i], useless_attr, (void*)f1, (void*) i)){
			printf(ANSI_COLOR_GREEN "Successfully created f1 pthread and enqueued. TID %ld\n" 
					ANSI_COLOR_RESET, pthread_array[i].threadID);
		}
	} 



	printf("\nPrinting priority array 0 (Inside main).  Should include pthreads 2 to 6.\n");
	_print_thread_list(scheduler->priority_array[0]);


	/* Main joins on thread2 */


	while(1){
		usleep(500000);
		printf("\tExecuting main!\n");
	}
}




void _debugging_pthread_mutex(int num){


  	struct timeval start, end;
  	gettimeofday(&start, NULL);


	printf(ANSI_COLOR_RED "\n\nRunning pthread_join() debug test...\n\n" ANSI_COLOR_RESET);
	
	int NUM_PTHREADS = num;


	// my_pthread_t pthread_array[NUM_PTHREADS];
	my_pthread_t* pthread_array = (my_pthread_t*)malloc(NUM_PTHREADS * sizeof(my_pthread_t));
	my_pthread_attr_t* useless_attr;
	my_pthread_mutexattr_t* useless_mattr;

	my_pthread_mutex_init(&mutex, useless_mattr);

	int i;

	for(i=0; i<NUM_PTHREADS;i++){

		/* TID all: lock mutex 1.  increment global counter.  unlock.  exit */
		if(my_pthread_create(&pthread_array[i], useless_attr, (void*)m1, (void*) &pthread_array[i])){
			printf(ANSI_COLOR_GREEN "Successfully created m1 pthread and enqueued. TID %ld\n" 
				ANSI_COLOR_RESET, pthread_array[i].threadID);
		}

		
	} 



	/* Main joins all pthreads */
	for(i=0; i<NUM_PTHREADS;i++){
		my_pthread_join(pthread_array[i], NULL);
	}	
	// my_pthread_join(pthread_array[NUM_PTHREADS-1], NULL);
	



  	gettimeofday(&end, NULL);

  	long int total_time = (end.tv_sec*1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);


	
	if(test_counter == num){
		printf(ANSI_COLOR_GREEN"\nSuccessful run with %i threads.\n"ANSI_COLOR_RESET, num);
	}else{
		printf(ANSI_COLOR_RED"\nFailure. Counter is %i but expected %i\n"
			ANSI_COLOR_RESET, test_counter, num);
	}

	// while(1){
	// 	usleep(500000);
	// 	printf("\tExecuting main!\n");
	// }






	printf("Total run time is: %ld microseconds.\n", total_time);
	printf(ANSI_COLOR_GREEN"Safely ending.\n"ANSI_COLOR_RESET); 

}