/************************************************************************************************************
*	cs518 - assignment01 
*	thread scheduling 
*
*	my_pthread_t.c 	
************************************************************************************************************/

#include "my_pthread_t.h"
#include "thread_unit_lib.h" 	

static scheduler_t* scheduler;

int 				SYS_MODE;
int					mutex_count = 0;
thread_unit*		maintenance_thread_unit;
thread_unit* 		main_thread_unit;
int 				first_run_complete;



/************************************************************************************************************
*
*    SCHEDULER LIBRARY 
*
************************************************************************************************************/

void priority_level_sort(){
	int i;
	int count = 0;
	thread_unit* iter = NULL;
	thread_unit* current = NULL;
	
	for(i = 0; i < PRIORITY_LEVELS;i++){
		current = scheduler->priority_array[i]->head; //
		iter 	= scheduler->priority_array[i]->iter;
		if(current == NULL){
			printf("Priority %d is empty\n",i);
			continue;
		}
		current->next = NULL;
		printf("Sorting priority %d \n",i);
		while(iter != NULL){
			current = scheduler->priority_array[i]->head;
			if(iter->run_count < scheduler->priority_array[i]->head->run_count){
				iter->next = scheduler->priority_array[i]->head;
				scheduler->priority_array[i]->head = iter;	
			}else {
				while(current->next != NULL && current->next->run_count < iter->run_count){
					current = current->next;
				}
				iter->next = current->next;
				current->next = iter;
			}
		}
		current = scheduler->priority_array[i]->head;
		while(current != NULL){
			printf("TID %d: run_count: %d \n",current->thread->threadID,current->run_count);
			if(current->next == NULL){
				scheduler->priority_array[i]->tail = current;
			}
			current = current->next;
		}
		scheduler->priority_array[i]->iter = scheduler->priority_array[i]->head->next; 
	}
	
}



void scheduler_runThread(thread_unit* thread, thread_unit* prev){



	if(thread == NULL){
		printf(ANSI_COLOR_RED "Attempted to schedule a NULL thread\n"ANSI_COLOR_RESET);
		return;
	}

	// if(scheduler->currently_running->state == RUNNING){
	// 	scheduler->currently_running->state = READY;
	// }

	
	scheduler->currently_running 		= thread;
	// scheduler->currently_running->state = RUNNING;

	/* Will execute function that thread points to. */ 
	

	if(prev == NULL && first_run_complete == 0){
		// Intialize case
		first_run_complete = 1; 
		if ((swapcontext(main_thread_unit->ucontext, thread->ucontext)) < 0)
			printf("Errno value %d:  Message: %s: Line %d\n", errno, strerror(errno), __LINE__);
	}
	else if(prev == NULL && first_run_complete == 1){
		if ((swapcontext(maintenance_thread_unit->ucontext, thread->ucontext)) < 0)
			printf("Errno value %d:  Message: %s: Line %d\n", errno, strerror(errno), __LINE__);

	}
	else{
		if ((swapcontext(prev->ucontext, thread->ucontext)) < 0)
			printf("Errno value %d:  Message: %s: Line %d\n", errno, strerror(errno), __LINE__);
	}

	/* Context will switch back to scheduler either when thread completes or at a SIGARLM */
}

void scheduler_sig_handler(){


	if(SYS_MODE == 1){
		return;
	}

    
    if(scheduler->currently_running != NULL){
    	printf(ANSI_COLOR_GREEN "\nScheduler Signal Handler:\n\tThread %ld just ran for %i microseconds. Interrupting...\n" 
    			ANSI_COLOR_RESET, 
    			scheduler->currently_running->thread->threadID, 
    			scheduler->currently_running->time_slice);
		
    }else{
    	printf(ANSI_COLOR_GREEN "\nScheduler Signal Handler:\n\tNo threads have run yet.\n" ANSI_COLOR_RESET);

    }
    

    my_pthread_yield();

}

/*
	TODO

	Right now, the maint cycle is set to run whenever the running queue runs out.

	I wrote in some code to try testing the scheduler if all the threads were in
	the top priority queue since i havent written out any of the priority degredation 
	stuff yet.  Thats why the for loop only checks priority_array[0] for now.

	This test seems to work.  I will stress test it more later. 

*/
void maintenance_cycle(){

	printf(ANSI_COLOR_YELLOW "\n----------------" ANSI_COLOR_RESET);
	printf(ANSI_COLOR_YELLOW "\nMaintenance Cycle\n" ANSI_COLOR_RESET);

	/**********************************************************************************
		Priority Adjustments	 TODO:  all of this 
	**********************************************************************************/



	/**********************************************************************************
		ADD RUNNING BACK IN 	 
	**********************************************************************************/
	
	thread_unit* temp;

	//scheduler->running->iter = scheduler->running->head;
	printf(ANSI_COLOR_YELLOW "The old running queue:\n"ANSI_COLOR_RESET);
	_print_thread_list(scheduler->running);


	while(!thread_list_isempty(scheduler->running)){

		if((temp = thread_list_dequeue(scheduler->running)) != NULL){

			if(temp->state == READY){
				thread_list_enqueue(scheduler->priority_array[0], temp);
			}
		
		}
	}



	/**********************************************************************************
		Populate running queue 	TODO:  Fix this shit 
	**********************************************************************************/

	/* Collect MAINT_CYCLE number processes to run and put them into running queue */
	int i; 
	scheduler->running->head = NULL;
	scheduler->running->tail = NULL;
	scheduler->running->iter = NULL;


	for(i=0; i<MAINT_CYCLE; i++){
		// Run until you add a thread to running queue 

		// Check all levels of priority_array
		thread_unit* temp;

		if((temp = thread_list_dequeue(scheduler->priority_array[0])) != NULL){

			thread_list_enqueue(scheduler->running, temp);

		}

	}



	/* Test prints */
	printf(ANSI_COLOR_YELLOW "The new running queue:\n" ANSI_COLOR_RESET);
	_print_thread_list(scheduler->running);


	printf(ANSI_COLOR_YELLOW "The current priority[0] queue:\n" ANSI_COLOR_RESET);
	_print_thread_list(scheduler->priority_array[0]);

	printf(ANSI_COLOR_YELLOW "----------------\n\n" ANSI_COLOR_RESET);


}


void scheduler_init(){

	// first_schedule_done = 0;

    /**********************************************************************************
		Initialize scheduler structures  
    **********************************************************************************/

	int i;

	first_run_complete = 0;

	/* Attempt to malloc space for scheduler */
	if ((scheduler = (scheduler_t*)malloc(sizeof(scheduler_t))) == NULL){
		printf("Errno value %d:  Message: %s: Line %d\n", errno, strerror(errno), __LINE__);
	}

	scheduler->threadID_count 		= 2;	// Reserve 0 and 1 for scheduler and main threads (just in case)
	scheduler->currently_running 	= NULL;

	/* Initialize each priority queue (thread_unit_list) */
	for(i = 0; i<= PRIORITY_LEVELS; i++){
		scheduler->priority_array[i] = thread_list_init();	// thread_list_init() mallocs
	}

	/* Initialize the currently running and waiting queues */
	scheduler->running = thread_list_init();
	scheduler->waiting = thread_list_init();
	


	/* TODO: build ucontext of the scheduler (func* to scheduler_sig_handler) */

	if((main_thread_unit = (thread_unit*)malloc(sizeof(thread_unit))) == NULL){
		printf("Errno value %d:  Message: %s: Line %d\n", errno, strerror(errno), __LINE__);
		exit(-1);		
	}

	if((maintenance_thread_unit = (thread_unit*)malloc(sizeof(thread_unit))) == NULL){
		printf("Errno value %d:  Message: %s: Line %d\n", errno, strerror(errno), __LINE__);
		exit(-1);		
	}

	main_thread_unit->state = READY;
	main_thread_unit->time_slice = TIME_QUANTUM;
	main_thread_unit->run_count = 0;
	main_thread_unit->wait_next = thread_list_init();
	main_thread_unit->next = NULL;

	// maintenance_thread_unit->state	= READY; 





    /**********************************************************************************
		MAIN_UCONTEXT SETUP   
    **********************************************************************************/

	/* Attempt to malloc space for main_ucontext */
	if ((main_thread_unit->ucontext = (ucontext_t*)malloc(sizeof(ucontext_t))) == NULL){
		printf("Errno value %d:  Message: %s: Line %d\n", errno, strerror(errno), __LINE__);
		exit(-1);
	}

	/* copy (fork) the current ucontext */
	if(getcontext(main_thread_unit->ucontext) == -1){
		printf("Error obtaining ucontext of scheduler");
		exit(-1);
	} 

	/* Set up ucontext stack */
	if((main_thread_unit->ucontext->uc_stack.ss_sp = malloc(PAGE_SIZE))==NULL){
		printf("Errno value %d:  Message: %s: Line %d\n", errno, strerror(errno), __LINE__);
		exit(-1);
	}
	main_thread_unit->ucontext->uc_stack.ss_size 	= PAGE_SIZE;

	/* Set uc_link to point to addr of main_ucontext */
	main_thread_unit->ucontext->uc_link 			= main_thread_unit->ucontext;

	/* Assign func* to ucontext */
	makecontext(main_thread_unit->ucontext, (void*)scheduler_sig_handler, 1, NULL); 	// Should we write a separate scheduler_run_thread call?
    
    /**********************************************************************************/

	/**********************************************************************************
		MAINTENANCE_UCONTEXT SETUP   
    **********************************************************************************/

	/* Attempt to malloc space for maintenance_ucontext */
	if ((maintenance_thread_unit->ucontext = (ucontext_t*)malloc(sizeof(ucontext_t))) == NULL){
		printf("Errno value %d:  Message: %s: Line %d\n", errno, strerror(errno), __LINE__);
		exit(-1);
	}

	/* copy (fork) the current ucontext */
	if(getcontext(maintenance_thread_unit->ucontext) == -1){
		printf("Error obtaining ucontext of scheduler");
		exit(-1);
	} 

	/* Set up ucontext stack */
	if((maintenance_thread_unit->ucontext->uc_stack.ss_sp = malloc(PAGE_SIZE))==NULL){
		printf("Errno value %d:  Message: %s: Line %d\n", errno, strerror(errno), __LINE__);
		exit(-1);
	}
	maintenance_thread_unit->ucontext->uc_stack.ss_size 	= PAGE_SIZE;


	/* Set uc_link to point to addr of maintenance_ucontext */
	maintenance_thread_unit->ucontext->uc_link 			= main_thread_unit->ucontext;

	/* Assign func* to ucontext */
	makecontext(maintenance_thread_unit->ucontext, (void*)maintenance_cycle, 1, NULL); 	// Should we write a separate scheduler_run_thread call?
     
    /**********************************************************************************/




	/*
		TODO: 
		Create thread_units for main thread and scheduler/maintenance thread
			Similar to thread_unit_init(pthread ptr*) except these two do not have
			pthreads. 
	*/




	scheduler->initialized = 1;



    /**********************************************************************************
		Initialize timer signal handler  
    **********************************************************************************/

	/* The below is useful for redirecting sigactions before setting a signal handler */
	// struct sigaction signal_action;
	// sigemptyset(&signal_action.sa_mask);

	// if(sigaction(SIGSEGV, &signal_action, NULL) == -1){
	// 	printf("Failure to initialize signal handler.\n");
	// 	exit(EXIT_FAILURE);
	// }


	/* Direct sig-alarms to scheduler_sig_handler */
	signal(SIGALRM, &scheduler_sig_handler);
	scheduler_sig_handler();

}



/************************************************************************************************************
*
*    MY_PTHREAD CORE LIBRARY 
*
************************************************************************************************************/


int my_pthread_create( my_pthread_t * thread, my_pthread_attr_t * attr, void *(*function)(void*), void * arg){

	/**********************************************************************************
		PTHREAD_T & THREAD_UNIT SETUP   
    **********************************************************************************/

	/* ASSUME PTHREAD ALREADY HAS SPACE ALLOCATED TO IT FROM USER */
	SYS_MODE = 1;

	// if ((thread = (my_pthread_t*)malloc(sizeof(my_pthread_t))) == NULL){
	// 	printf("Errno value %d:  Message: %s: Line %d\n", errno, strerror(errno), __LINE__);
	// }

	thread->threadID = scheduler->threadID_count;
	scheduler->threadID_count++; 	// Uses some global counter for threadID.  Change method later 

	thread->return_val = NULL;


	/* Init thread_unit for this pthread */
	thread_unit* new_unit 				= thread_unit_init(thread);			//thread_unit_init() mallocs
	
	/* pthread struct holds a pointer to it's containing thread_unit struct */
	thread->thread_unit 				= new_unit;

    /**********************************************************************************
		UCONTEXT SETUP   
    **********************************************************************************/

	/* copy (fork) the current ucontext */
	if(getcontext(new_unit->ucontext) < 0){
		printf("Error obtaining ucontext of thread");
		exit(-1);
	} 

	/* Set up ucontext stack */
	if((new_unit->ucontext->uc_stack.ss_sp = malloc(PAGE_SIZE))==NULL){
		printf("Errno value %d:  Message: %s: Line %d\n", errno, strerror(errno), __LINE__);
	}
	new_unit->ucontext->uc_stack.ss_size 	= PAGE_SIZE;
		/* wtf is happening here^?  I found this online somewhere.  Is it working?*/


	/* Set uc_link to point to addr of scheduler's ucontext */
	new_unit->ucontext->uc_link 			= maintenance_thread_unit->ucontext;

	/* Assign func* to ucontext */
	makecontext(new_unit->ucontext, (void*)function, 1, arg); 		
	// Should we write a separate scheduler_run_thread call?
    // errno for < 0 (be sure to cleanup if failure)


    /**********************************************************************************
		SCHEDULE THAT SHIT    
    **********************************************************************************/
	// into highest priority bc heuristics say so 

	new_unit->priority 	= 0;
	new_unit->state 	= READY;
	thread_list_enqueue(scheduler->priority_array[0], new_unit);

	

	/*
		By the end of my_pthread_create():
			The incoming pthread ptr will be populated with defaults
			That pthread will be put into a thread_unit_t
			That thread_unit_t's ucontext will be populated 
			That thread_unit will be enqueued into the priority 0 thread list.
		End by calling yield().
	*/

	SYS_MODE = 0;
	
	// my_pthread_yield();
	

	return 1;
}



void my_pthread_yield(){
	/* Run next thread in the running queue.  If the running queue is empty, run maint_cycle. */

	SYS_MODE = 1;

	struct itimerval 	timer;
	thread_unit* 		thread_up_next = NULL;
	thread_unit* 		prev = scheduler->currently_running;

	/* Currently_runnig is the one that just finished running */
	if(!thread_list_isempty(scheduler->running) && scheduler->currently_running != NULL){
		scheduler->currently_running->state = READY;
		scheduler->currently_running->run_count++;
	}


	if(scheduler->running->iter == NULL){
		// End of running queue.  Run maint cycle.
		maintenance_cycle(); 
	}

	if(!thread_list_isempty(scheduler->running)){

		thread_up_next						= scheduler->running->iter;
		scheduler->running->iter 			= scheduler->running->iter->next;
		scheduler->currently_running 		= thread_up_next;
		thread_up_next->state 				= RUNNING;
		// thread_up_next->run_count++;
    	timer.it_value.tv_usec 				= scheduler->currently_running->time_slice;	// "" (50 ms)

		printf(ANSI_COLOR_GREEN "\tSwitching to Thread %ld...\n"ANSI_COLOR_RESET, thread_up_next->thread->threadID);
	}else{
	    timer.it_value.tv_usec 		= TIME_QUANTUM;	// wait 50ms if running queue is empty. 

	}


	/* Set timer */
    timer.it_value.tv_sec 		= 2;			// Time remaining until next expiration (sec)
    timer.it_interval.tv_sec 	= 0;			// Interval for periodic timer (sec)
    timer.it_interval.tv_usec 	= 0;			// "" (microseconds)
    setitimer(ITIMER_REAL, &timer, NULL);


	SYS_MODE = 0;

    /* Run next thread in line */
	scheduler_runThread(thread_up_next, prev);


}




void my_pthread_exit(void *value_ptr){

	/*
		TODO	
		As per discussion:
			set id = -1
			dont actually remove.  let the maintcycle do it.

		So the below is mostly incorrect.  Do not want to free yet.
		Only pass the void* along ad then call yield
	*/


	SYS_MODE = 1;

	if(scheduler->currently_running->state == TERMINATED){
		printf("This thread, TID %ld, has already been terminated.\n", scheduler->currently_running->thread->threadID);
	}

	scheduler->currently_running->state = TERMINATED;
	scheduler->currently_running->thread->threadID = -1;
	/*
		Now we loop through the waiting on me list to store the return val into 
		the return_val for threads that had joined it
	
	*/
	//scheduler->currently_running->thread->return_val = value_ptr;
	thread_unit_* temp = scheduler->waiting->head;
	while(temp != NULL){
		if(temp->joinedID == scheduler->currently_running->thread->threadID){
			
	}
	/* Free malloced memory and cleanup */
	free(scheduler->currently_running->thread);
	free(scheduler->currently_running->ucontext->uc_stack.ss_sp);
	free(scheduler->currently_running->ucontext);
	//	free(scheduler->currently_running);  // Keep the thread_unit around for scheduler book keeping 

	my_pthread_yield();

}



int my_pthread_join(my_pthread_t thread, void **value_ptr){

	/* 
		TODO

		Problem.  The parameter to this function is just a pthread struct.
		We might need to store the state of the thread inside the pthread_t
		to get this function to work.

		I hashed out what I think the function would look like below, but it 
		requires state to be stored in pthread_t.  

	*/
	if(thread->thread_unit == NULL){
		printf("Thread to join does not exist\n");
		return -1;
	}
	if(scheduler->current_running->thread->threadID = thread->threadID){
		printf("Trying to join itself\n");
		return -1;
	}
	if(scheduler->current_running->state != RUNNING){
		printf("Not possible to join because I am not ready\n");
		return -1;
	}
	thread_unit * temp = scheduler->current_running->wait_next->head;
	while(temp != NULL){
		temp = temp->


	// while(thread.state != TERMINATED){
	// 	my_pthread_yield();
	// }

	// thread->return_val = value_ptr;

}


/************************************************************************************************************
*
*    MY_PTHREAD_MUTEX LIBRARY 
*
************************************************************************************************************/

int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const my_pthread_mutexattr_t *mutexattr){
	SYS_MODE = 1;
	if(mutex->initialized == 1){
		printf("Mutex is already initialized\n");
		return -1;
	}
	mutex->initialized = 1;
	mutex->id = mutex_count;
	mutex_count++;
	mutex->waiting_queue = NULL;
	resetTimer();
	return 0;
}
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex){
	//idea here is to pass ownership of the lock to next in queue instead of checking if the lock is available
	//only do we actually set the lock to 1 once we call lock for an empty queue
	

	thread_unit_* temp = mutex->waiting_queue;
	if(mutex->initialized == 0){
		printf("Trying to lock an uninitialized mutex\n");
		return -1;
	}
	if(mutex->lock == 1 && mutex->owner == scheduler->currently_running->thread->threadID){
		printf("You own the lock, why are you trying to lock it?\n");
		return -1;
	}
	if(mutex->lock == 1){
		// Lock is currently in use & wait_queue is empty 
		SYS_MODE = 1;
		printf("lock is in use, adding myself to the mutex waiting queue for lock %d\n",mutex->lock); 
		if(temp == NULL){
			mutex->waiting_queue = scheduler->currently_running;
			scheduler->currently_running->state == WAITING;
			pthread_yield();
			//when thread resumes, it should be getting the lock
			mutex->lock = 1;
			mutex->owner = scheduler->currently_running->thread->threadID;
			return 0;
		}

		while(temp->mutex_next != NULL){ 
		//add thread to the end of the queue
		//use a thread list here?  but then we need to create functions using mutex_next instead of next
			temp = temp->mutex_next;
		}
		temp->mutex_next = scheduler->currently_running;
		scheduler->currently_running->state = WAITING;
		pthread_yield();
		mutex->lock = 1;
		mutex->owner = scheduler->currently_running->thread->threadID;
		return 0;
	}

	/* Lock says its available, but its waiting_queue is not empty  */
	if(mutex->lock == 0 && mutex->waiting_queue != NULL){
		printf("wait your turn\n");
		SYS_MODE = 1;
		while(temp->mutex_next != NULL){
			temp = temp->mutex_next;
		}
		temp->mutex_next = scheduler->currently_running;
		scheduler->currently_running->state = WAITING;
		pthread_yield();
		mutex->lock = 1;
		mutex->owner = scheduler->currently_running->thread->threadID;
		return 0;
	}

	/* First person to claim lock */
	if(mutex->lock == 0 && mutex->waiting_queue == NULL){
		SYS_MODE = 1;
		mutex->lock = 1;
		mutex->owner = scheduler->currently_running->thread->threadID;
		resetTimer();
		return 0;
	}
	printf("panic\n");
	return -1;
}
/* Resets timer to 4 TQ */
void resetTimer(){

	int 				TIMER_MULTIPLE 	= 3;
	struct itimerval 	timer;
	thread_unit* 		thread_up_next = NULL;
	thread_unit* 		prev = scheduler->currently_running;

	/* Set timer */
    timer.it_value.tv_sec 		= 0;			// Time remaining until next expiration (sec)
    timer.it_value.tv_usec 		= TIMER_MULTIPLE*TIME_QUANTUM;	// wait 50ms if running queue is empty. 
    timer.it_interval.tv_sec 	= 0;			// Interval for periodic timer (sec)
    timer.it_interval.tv_usec 	= 0;			// "" (microseconds)
    setitimer(ITIMER_REAL, &timer, NULL);


	SYS_MODE = 0;
	return;
}

int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex){
	if(mutex->initialized == 0){
		printf("Trying to unlock an uninitialized mutex\n");
		return -1;
	}
	if(mutex->lock == 0){
		printf("nothing to unlock\n");
		return -1;
	}
	if(mutex->lock == 1 && mutex->owner != scheduler->currently_running->thread->threadID){
		printf("not the owner so not unlocking\n");
		return -1;
	}
	if(mutex->lock == 1 && mutex->owner == scheduler->currently_running->thread->threadID){
		
		SYS_MODE = 1;
		if(mutex->waiting_queue == NULL){
			mutex->lock = 0;
			mutex->owner = -1; // because thread 0 is main
			my_pthread_yield();
			return 0;
		}else{
			mutex->waiting_queue->state = READY;
			mutex->waiting_queue = mutex->waiting_queue->mutex_next;
			mutex->owner = -1;
			//leave it locked so nobody sneaks in for a steal
			my_pthread_yield();
			return 0;
		}
	}
	printf("panic\n");
	return -1;
}
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex){

	SYS_MODE = 1;
	if(mutex->initialized == 0){
		printf("Trying to destroy an uninitialized mutex\n");
		my_pthread_yield();
		return -1;
	}
	// if(mutex->lock == 1 && mutex->waiting_queue!= NULL){
	// 	printf("Cant destory while others are waiting for the lock\n");
	// 	//we should force release or we might hit a deadlock
	// 	my_pthread_mutex_unlock(&mutex);
	// 	return 0;
	// }
	if(mutex->lock == 1 && mutex->owner == scheduler->current_running->thread->threadID && mutex->waiting_queue == NULL){
		SYS_MODE =1;
		printf("you are the owner of the lock and can freely destroy it");
		mutex->initialized = 0;
		mutex->owner = -1;
		mutex->lock = 0;
		resetTimer();
		//no memory was allocated so its on the user to free
		return 0;
	}
	if(mutex->lock == 0 && mutex->owner == -1){
		SYS_MODE =1;
		printf("no one waiting for the lock so its safe to destroy\n");
		mutex->initialized = 0;
		//no memory was allocated so its on the user to free
		resetTimer();
		return 0;
	}

	//panic?
	printf("panic!\n");
	return 0;
}



	

/************************************************************************************************************
*
*    DEBUGGING / TESTING
*
************************************************************************************************************/

/*
	Function for testing
*/
void f1(int x){

	while(1){
		sleep(1);
		printf("\tExecuting f1:\tArg is %i\n", x);
	
	}

	my_pthread_exit(NULL);
}




void _debugging_pthread_yield(){


	printf(ANSI_COLOR_RED "\n\nRunning pthread_yield() debug test...\n\n" ANSI_COLOR_RESET);
	
	int NUM_PTHREADS = 5;


	/* init scheduler...calls sig handler */
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

	printf(ANSI_COLOR_RED "End pthread_create() debug test." ANSI_COLOR_RESET);

}



int main(){

	// _debugging_thread_unit_lib();
	// _debugging_pthread_create();

	_debugging_pthread_yield();

}