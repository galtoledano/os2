//
// Created by galtoledano on 16/04/2020.
//

#include "thread.h"


#ifdef __x86_64__
/* code for 64 bit Intel arch */

#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
                 "rol    $0x11,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}

#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
		"rol    $0x9,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#endif

/*
 * constractor for thread element, creating new thread including stack for the thread.
 */
thread::thread(int quantum, int id, address_t foo) : quantum(quantum), id(id),
            pc(foo), call(0), state(READY){
    char* stack_ = new char[STACK_SIZE];
    this->stack = stack_;
    sp = (address_t)(stack_) + STACK_SIZE - sizeof(address_t);
    sigsetjmp(env, 1);
    if(id!=0){
        (env->__jmpbuf)[JB_SP] = translate_address(sp);
        (env->__jmpbuf)[JB_PC] = translate_address(pc);
    }
    sigemptyset(&env->__saved_mask);

}

/*
 * return the thread's envelope
 */
sigjmp_buf *thread::getEnv(){
    return &env;
}

/*
 * return the current thread's quantum
 */
int thread::getQuantum() const {
    return quantum;
}

/*
 * return the current thread's amount of calling
 */
int thread::getCall() const {
    return call;
}

/*
 * return the thread's id
 */
int thread::getId() const {
    return id;
}

/*
 * return the current thread's state
 */
int thread::getState() const {
    return state;
}

/*
 * set new state for the thread
 */
void thread::setState(int state) {
    thread::state = state;
}

/*
 * set new number of quantum for the thread
 */
void thread::setQuantum(int quantum) {
    thread::quantum = quantum;
}



