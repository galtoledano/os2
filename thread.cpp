//
// Created by galtoledano on 16/04/2020.
//

#include "thread.h"

#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
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

thread::thread(int quantum, int id, void (*foo)(void)) : quantum(quantum), id(id)
{
    //todo : change
    int s = 16384;
    this->call = 0;
    this->f = foo;
    this->state = READY;
    char stack[s];
    this->sp = (address_t)stack + s - sizeof(address_t);
    this->pc = (address_t)f;
    sigsetjmp(env, 1);
    (env->__jmpbuf)[JB_SP] = translate_address(this->sp);
    (env->__jmpbuf)[JB_PC] = translate_address(this->pc);
    sigemptyset(&env->__saved_mask);

}

sigjmp_buf &thread::getEnv(){
    return env;
}

int thread::getQuantum() const {
    return quantum;
}

int thread::getId() const {
    return id;
}

int thread::getState() const {
    return state;
}

void thread::setState(int state) {
    thread::state = state;
}

void thread::setQuantum(int quantum) {
    thread::quantum = quantum;
}

int thread::getCall() const {
    return call;
}

