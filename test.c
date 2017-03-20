#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <signal.h>

#include <limits.h>

#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

int* p;


void signal_handler(int sig, siginfo_t* si, void* ptr){

	if(sig == SIGSEGV){
		int* addr = si->si_addr;

		printf("%p\n", addr);
		printf("%p\n", p);

		int diff = addr-p;

		printf("%d\n", diff);


		printf("We got a SIGSEGV signal\n");
		if(mprotect(p, 1024, PROT_WRITE)){
			perror("Couldn't mprotect - PROT_WRITE");
			exit(errno);
		}
	}
}


int main(void) {
	struct sigaction s;
	s.sa_flags = SA_SIGINFO|SA_RESETHAND;
	s.sa_sigaction = signal_handler;
	sigemptyset(&s.sa_mask);
	sigaction(SIGSEGV, &s, 0);


	// signal(SIGSEGV, &signal_handler);

	int i;
	for(i=5; i<5; i++){
		printf("This is run: %d", i);
	}
	printf("Done running loop.\n");

	int c;

	/* Allocate a buffer; it will have the default protection of PROT_READ|PROT_WRITE */
	p = (int*)malloc(1024);
	if(!p) {
		perror("Couldn't malloc(1024)");
		exit(errno);
	}

	/* Align to a multiple of PAGESIZE, assumed to be a power of two */
	p = (((int)p + PAGESIZE-1) & ~(PAGESIZE-1));

	c = p[1022];			/* Read; ok */
	p[1022] = 42;			/* Write; ok */
	printf("Our value is: %d\n", p[1022]);

	/* Mark the buffer not read or write */
	if(mprotect(p, 256, PROT_NONE)) {
		perror("Couldn't mprotect - PROT_NONE");
		exit(errno);
	}

	// c = p[1022];				/* Read; ok */
	// p[1022] = 42;			/* Write; program dies on SIGSEGV  */
	printf("Our value is: %d\n", p[1022]);


	printf("Finished running\n");
	exit(0);

}