#include <stdint.h>
#include <stdbool.h>
extern uint32_t *getstackptr();
extern bool osCreateThread(void (*ptr)(void*), void* arg);
extern bool osCreateThreadWithDeadline(void (*ptr)(void*), void* arg, uint32_t time);
extern void osKernelStart();
extern void osKernelInitialize();
extern void osSched();
extern void osYield(void);

#define SHPR2 *(uint32_t*)0xE000ED1C //for setting SVC priority, bits 31-24
#define SHPR3 *(uint32_t*)0xE000ED20 // PendSV is bits 23-16
#define _ICSR *(uint32_t*)0xE000ED04 //This lets us trigger PendSV

typedef struct k_thread{
uint32_t* sp; //stack pointer
void (*thread_function)(void*); //function pointer
uint32_t timeslice; // the number of milliseconds this thread is allowed to run for, presuming it does not yield beforehand
uint32_t runtime; // the number of milliseconds that this thread has left to run, once it gets started
}thread;
