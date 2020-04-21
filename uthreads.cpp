//
// Created by galtoledano on 16/04/2020.
//


#include <iostream>
#include <deque>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <setjmp.h>
#include <unistd.h>
#include "uthreads.h"
#include "thread.h"

typedef unsigned long address_t;

void empty(){}
struct sigaction sa = {0}; // todo : move back to reset timer ?
struct itimerval timer;
sigjmp_buf env[MAX_THREAD_NUM];  // todo : good ?
static int threads_counter = 1;
static int total_quant = 0;
thread* current_thread;
std::deque<thread*> tready;
std::vector<int> quantums_list;
std::vector<thread*> threads(MAX_THREAD_NUM);


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

void round_robin(int sig){
    if(tready.empty()){
       // todo : handle empty ready list
    }
    switch(current_thread->getState()){
        case RUN: {
            int ret_val = sigsetjmp(env[current_thread->getId()], 1);
            if(ret_val == 1){
                current_thread->setState(READY);
                tready.push_back(current_thread);
                current_thread = tready.front();
                tready.pop_front();
                timer.it_interval.tv_usec = current_thread->getQuantum();
                current_thread->inc_calls();
                total_quant++;
                return;
            }
            siglongjmp(env[current_thread->getId()], 1);
        }



        case BLOCK:{
            // todo : more ?
            uthread_block(current_thread->getId());
            return;
        }
    }
}

int reset_time(int quant){

    sa.sa_handler = &round_robin;
    if (sigaction(SIGVTALRM, &sa, nullptr) < 0) {
        std::cerr <<  "system error: sigaction error\n";
        return -1;
    }
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = quant;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = quant;
    if (setitimer (ITIMER_VIRTUAL, &timer, nullptr)) {
        std::cerr <<  "system error: setitimer error\n";
        return -1;
    }
    return 0;
}

int uthread_init(int *quantum_usecs, int size){
    for(int i=0; i<size; ++i){
        if(quantum_usecs[i] < 0){
            std::cerr << "thread library error: there is a negative quantum size\n";
            return -1;
        }
    }
    //todo quantums_list = quantum_usecs;
    thread* main_thread = new thread(quantum_usecs[0], 0 , empty);
    // todo: the RR algorithm
    int threads_counter = 1;
    reset_time(main_thread->getQuantum());
    //thread* threads_list = new thread[MAX_THREAD_NUM];
    return 0;
}

/*
 * Description: This function creates a new thread, whose entry point is the
 * function f with the signature void f(void). The thread is added to the end
 * of the READY threads list. The uthread_spawn function should fail if it
 * would cause the number of concurrent threads to exceed the limit
 * (MAX_THREAD_NUM). Each thread should be allocated with a stack of size
 * STACK_SIZE bytes.
 * priority - The priority of the new thread.
 * Return value: On success, return the ID of the created thread.
 * On failure, return -1.
*/
int uthread_spawn(void (*f)(void), int priority){
    if (threads_counter >= MAX_THREAD_NUM){
        std::cerr << "thread library error: too many threads\n";
        return -1;
    }
    for (int i = 0; i < MAX_THREAD_NUM; ++i) {
        if (threads[i] == nullptr){
            auto t = new thread(quantums_list[priority], i, f);
            if(t == nullptr){
                std::cerr << "system error: can't alloc memory\n";
                return -1;
            }
            threads[i] = t;
            tready.push_back(t);
            threads_counter ++;
            return i;
        }
    }
    std::cerr << "thread library error: too many threads\n";
    return -1;
}

/*
 * Description: This function changes the priority of the thread with ID tid.
 * If this is the current running thread, the effect should take place only the
 * next time the thread gets scheduled.
 * Return value: On success, return 0. On failure, return -1.
*/
int uthread_change_priority(int tid, int priority){
    if(priority >= quantums_list.size()){
        std::cerr <<  "thread library error: wrong priority\n";
        return -1;
    }
    threads[tid]->setQuantum(quantums_list[priority]);
    return 0;
    //todo : current running thread ??
}

/*
 * Description: This function terminates the thread with ID tid and deletes
 * it from all relevant control structures. All the resources allocated by
 * the library for this thread should be released. If no thread with ID tid
 * exists it is considered an error. Terminating the main thread
 * (tid == 0) will result in the termination of the entire process using
 * exit(0) [after releasing the assigned library memory].
 * Return value: The function returns 0 if the thread was successfully
 * terminated and -1 otherwise. If a thread terminates itself or the main
 * thread is terminated, the function does not return.
*/
int uthread_terminate(int tid){
    if(tid == 0){
        for (int i = 0; i < MAX_THREAD_NUM; ++i) {
            if(threads[i] != nullptr){
                delete(threads[i]);
                threads[i] = nullptr;
            }
            //todo like this ?
            threads.clear();
        }
        //todo : stop pointing on the threads from thr queues!
        exit(0);
    }
    else{
        int s = threads[tid]->getState();
        //todo : delete from the state queue !
        threads[tid] = nullptr;
        threads_counter--;
        return 0;
    }
    // todo thread terminates itself ???
    std::cerr <<  "thread library error: can't delete the thread\n";
    return -1;
}


/*
 * Description: This function blocks the thread with ID tid. The thread may
 * be resumed later using uthread_resume. If no thread with ID tid exists it
 * is considered as an error. In addition, it is an error to try blocking the
 * main thread (tid == 0). If a thread blocks itself, a scheduling decision
 * should be made. Blocking a thread in BLOCKED state has no
 * effect and is not considered an error.
 * Return value: On success, return 0. On failure, return -1.
*/
int uthread_block(int tid){
    if(threads[tid] == nullptr){
        std::cerr <<  "thread library error: no thread like that\n";
        return -1;
    }
    if(tid == 0)
    {
        std::cerr <<  "thread library error: can't block the main thread\n";
        return -1;
    }
    int s = threads[tid]->getState();
    if(s == READY){
//        std::deque<threads*>::iterator it =std::find(tready.begin(), tready.end(), threads[tid]);
//        if(it!=threads.end()) {
//            it = tready.erase(it);
//        }
        tready.erase(std::remove(tready.begin(), tready.end(), threads[tid]), tready.end());
        threads[tid]->setState(BLOCK);
    }
    if(s == RUN){
        //todo : stop the run !
        threads[tid]->setState(BLOCK);
    }
    // todo: thread blocks itself ??
    return 0;
}
/*
 * Description: This function resumes a blocked thread with ID tid and moves
 * it to the READY state if it's not synced. Resuming a thread in a RUNNING or READY state
 * has no effect and is not considered as an error. If no thread with
 * ID tid exists it is considered an error.
 * Return value: On success, return 0. On failure, return -1.
*/
int uthread_resume(int tid){
    if(threads[tid] == nullptr){
        std::cerr <<  "thread library error: no thread like that\n";
        return -1;
    }
    int s = threads[tid]->getState();
    if(s == BLOCK){
        tready.push_back(threads[tid]);
        threads[tid]->setState(READY);
    }
    return 0;
}

/*
 * Description: This function returns the thread ID of the calling thread.
 * Return value: The ID of the calling thread.
*/
int uthread_get_tid(){
    return current_thread->getId();
}

/*
 * Description: This function returns the total number of quantums since
 * the library was initialized, including the current quantum.
 * Right after the call to uthread_init, the value should be 1.
 * Each time a new quantum starts, regardless of the reason, this number
 * should be increased by 1.
 * Return value: The total number of quantums.
*/
int uthread_get_total_quantums(){
    return total_quant;
}

/*
 * Description: This function returns the number of quantums the thread with
 * ID tid was in RUNNING state. On the first time a thread runs, the function
 * should return 1. Every additional quantum that the thread starts should
 * increase this value by 1 (so if the thread with ID tid is in RUNNING state
 * when this function is called, include also the current quantum). If no
 * thread with ID tid exists it is considered an error.
 * Return value: On success, return the number of quantums of the thread with ID tid.
 * 			     On failure, return -1.
*/
int uthread_get_quantums(int tid){
    // todo: fail ? 
    return threads[tid]->getCall();
}

