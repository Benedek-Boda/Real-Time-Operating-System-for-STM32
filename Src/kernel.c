#include "kernel.h"
#include <stdint.h>
#include <stdlib.h>
#include<stdio.h>
#include <stdbool.h>
#include "main.h"
#define stack_size 0x400
#define RUN_FIRST_THREAD 0x3
#define MAX_THREADS_ALLOWED 15
#define YIELD 55
extern void runFirstThread(void);

uint32_t* MSP_INIT_VAL;
uint32_t*stackptr;

uint32_t* current_stack;


//thread mythread;

thread threads[MAX_THREADS_ALLOWED];
unsigned int currthreadindex;
unsigned int numThreadsRunning;


uint32_t *getstackptr(){
	if ((void*)current_stack > (void*)MSP_INIT_VAL - (0x4000 - stack_size)){
	current_stack = (void*)current_stack - stack_size;
	uint32_t *ptr = current_stack;
	return ptr;
	} else{
	uint32_t *ptr = NULL;
	return ptr;
	}
}

void osSched()
{
	threads[currthreadindex].sp = (uint32_t*)(__get_PSP() - 8*4);

	currthreadindex = (currthreadindex+1)%numThreadsRunning;

	__set_PSP((uint32_t)threads[currthreadindex].sp);


}

void osYield(void){
	__asm("SVC #55");
}


void osKernelInitialize(){
	MSP_INIT_VAL = *(uint32_t**)0x0;
	current_stack = MSP_INIT_VAL;
	currthreadindex = 0;
	numThreadsRunning = 0;
	//set the priority of PendSV to almost the weakest
	SHPR3 |= 0xFE << 16; //shift the constant 0xFE 16 bits to set PendSV priority
	SHPR2 |= 0xFDU << 24; //Set the priority of SVC higher than PendSV
}

void osKernelStart(){
	__asm("SVC #0x3");
}

bool osCreateThread(void (*ptr)(void*), void* arg){
	stackptr = getstackptr();
	if(stackptr == NULL){
		return false;
	}

	  *(--stackptr) = 1<<24; //A magic number, this is xPS
	  *(--stackptr) = (uint32_t)ptr; //the function name
	  *(--stackptr) = 0xA; //An arbitrary number
	  *(--stackptr) = 0xA; //An arbitrary number
	  *(--stackptr) = 0xA; //An arbitrary number
	  *(--stackptr) = 0xA; //An arbitrary number
	  *(--stackptr) = 0xA; //An arbitrary number
	  *(--stackptr) = (uint32_t)arg;
	  *(--stackptr) = 0xA; //An arbitrary number
	  *(--stackptr) = 0xA; //An arbitrary number
	  *(--stackptr) = 0xA; //An arbitrary number
	  *(--stackptr) = 0xA; //An arbitrary number
	  *(--stackptr) = 0xA; //An arbitrary number
	  *(--stackptr) = 0xA; //An arbitrary number
	  *(--stackptr) = 0xA; //An arbitrary number
	  *(--stackptr) = 0xA; //An arbitrary number

	  threads[ numThreadsRunning].sp = stackptr;
	  threads[ numThreadsRunning].thread_function = ptr;
	  threads[ numThreadsRunning].timeslice = 5;
	  threads[ numThreadsRunning].runtime = 5;
	  numThreadsRunning++;


	  return true;
}

bool osCreateThreadWithDeadline(void (*ptr)(void*), void* arg, uint32_t time){
	stackptr = getstackptr();
	if(stackptr == NULL){
		return false;
	}

	  *(--stackptr) = 1<<24; //A magic number, this is xPS
	  *(--stackptr) = (uint32_t)ptr; //the function name
	  *(--stackptr) = 0xA; //An arbitrary number
	  *(--stackptr) = 0xA; //An arbitrary number
	  *(--stackptr) = 0xA; //An arbitrary number
	  *(--stackptr) = 0xA; //An arbitrary number
	  *(--stackptr) = 0xA; //An arbitrary number
	  *(--stackptr) = (uint32_t)arg;
	  *(--stackptr) = 0xA; //An arbitrary number
	  *(--stackptr) = 0xA; //An arbitrary number
	  *(--stackptr) = 0xA; //An arbitrary number
	  *(--stackptr) = 0xA; //An arbitrary number
	  *(--stackptr) = 0xA; //An arbitrary number
	  *(--stackptr) = 0xA; //An arbitrary number
	  *(--stackptr) = 0xA; //An arbitrary number
	  *(--stackptr) = 0xA; //An arbitrary number

	  threads[ numThreadsRunning].sp = stackptr;
	  threads[ numThreadsRunning].thread_function = ptr;
	  threads[ numThreadsRunning].timeslice = time;
	  threads[ numThreadsRunning].runtime = time;
	  numThreadsRunning++;

	  return true;
}

void SVC_Handler_Main( unsigned int *svc_args )
{
unsigned int svc_number;
/*
* Stack contains:
* r0, r1, r2, r3, r12, r14, the return address and xPSR
* First argument (r0) is svc_args[0]
*/
svc_number = ( ( char * )svc_args[ 6 ] )[ -2 ] ;
switch( svc_number )
{
case 17: //17 is sort of arbitrarily chosen
printf("Success!\r\n");
break;
case 15:
	printf("MTE 241\r\n");
	break;
case 5:
	printf("STM32CubeIDE\r\n");
break;
case RUN_FIRST_THREAD:
	__set_PSP((uint32_t)threads[currthreadindex].sp);
	runFirstThread();
	break;
case YIELD:
		threads[ currthreadindex].runtime = threads[ currthreadindex].timeslice;
		//Pend an interrupt to do the context switch
		_ICSR |= 1<<28;
		__asm("isb");
		break;
default: /* unknown SVC */
break;
}
}
