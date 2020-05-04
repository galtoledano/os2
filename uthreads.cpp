//
// Created by galtoledano on 16/04/2020.
//


#include <iostream>
#include <deque>
#include <vector>
#include <algorithm>
#include "uthreads.h"
#include "thread.h"



typedef unsigned long address_t;
struct sigaction sa;
struct itimerval timer;
sigset_t sigSet;

/* counters */
static int threads_counter = 0;
static int total_quant = 0;
int current_thread = 0;

/*data structures for the library*/
int* quantums_list;
int list_size;
std::deque<int> tready;
std::vector<thread*> threads (MAX_THREAD_NUM);

/*
 * block or unblock signals
 */
bool handle_signals(int state){
    if(sigprocmask(state, &sigSet, NULL) == -1){
        std::cerr << "System error : failed at blocking signals"<< std::endl;
        return false;
    }
    return true;
}

/*
 * promoting the first thread at the queue to be the running thread.
 */
void ready_to_run(){
    current_thread = tready.front();
    tready.pop_front();
    threads[current_thread]->setState(RUN);
    threads[current_thread]->inc_calls();
}

/*
 * reset the timer according to the given quantom
 */
int reset_time(int quant){
    timer.it_value.tv_sec = quant / SECOND;
    timer.it_value.tv_usec = quant % SECOND;
    timer.it_interval.tv_sec = quant / SECOND;
    timer.it_interval.tv_usec = quant % SECOND;
    if (setitimer (ITIMER_VIRTUAL, &timer, nullptr) < 0) {
        std::cerr <<  "system error: setitimer error\n";
        return -1;
    }
    return 0;
}

/*
 * the round robin algorithm function
 */
void round_robin(bool is_block = false){
    if(!handle_signals(SIG_BLOCK)){return;}

    // the tready queue is empty
    if(tready.empty()){
        threads[current_thread]->inc_calls();
        reset_time(threads.at(current_thread)->getQuantum());
        total_quant ++;
        handle_signals(SIG_UNBLOCK);
        return;
    }
    // handle with terminated thread
    else if(threads.at(current_thread) == nullptr){
        ready_to_run();
    }
    else {
        int ret_val = sigsetjmp(*threads.at(current_thread)->getEnv(), 1);
        if(ret_val == 0) {
            int temp = current_thread;
            ready_to_run();
            if(!is_block){
                threads[temp]->setState(READY);
                tready.push_back(temp);
            }
        }
        else{
            handle_signals(SIG_UNBLOCK);
            return;
        }
    }
    total_quant++;
    reset_time(threads.at(current_thread)->getQuantum());
    handle_signals(SIG_UNBLOCK);
    siglongjmp(*(threads.at(current_thread)->getEnv()), 1);
    return;
}

/*
 * the time handler, run at the end of the timer.
 */
void time_handler(int sig){
    if(sig == SIGVTALRM || sig == 0){
        round_robin();
    }
}

/*
 * init the program's signals.
 */
void init_signals(){
    sa.sa_handler = &time_handler;
    sa.sa_flags = 0;

    if(sigemptyset(&sigSet) == -1){
        std::cerr <<  "system error: sigemptyset error\n";
        exit(1);
    }
    if(sigaddset(&sigSet, SIGVTALRM) == -1){
        std::cerr <<  "system error: error at adding signal\n";
        exit(1);
    }
    if (sigaction(SIGVTALRM, &sa, NULL) < 0) {
        std::cerr <<  "system error: sigaction error\n";
        exit(1);
    }
    if(sigemptyset(&sa.sa_mask) == -1){
    std::cerr <<  "system error: sigemptyset error\n";
    exit(1);
    }
}

/*
 * Description: This function initializes the thread library.
 * You may assume that this function is called before any other thread library
 * function, and that it is called exactly once. The input to the function is
 * an array of the length of a quantum in micro-seconds for each priority.
 * It is an error to call this function with an array containing non-positive integer.
 * size - is the size of the array.
 * Return value: On success, return 0. On failure, return -1.
*/
int uthread_init(int *quantum_usecs, int size){
    if(size <= 0)
    {
        std::cerr << "thread library error: invalid input\n";
        return -1;
    }
    for(int i=0; i<size; ++i){
        if(quantum_usecs[i] <= 0){
            std::cerr << "thread library error: there is a negative quantum size\n";
            return -1;
        }
    }

    //init main thread
    quantums_list = quantum_usecs;
    list_size = size;

    threads.at(0) = new thread(quantum_usecs[0], 0 , (address_t)(nullptr));
    threads.at(0)->setState(RUN);
    threads.at(0)->inc_calls();

    // init signal set
    init_signals();

    reset_time(threads.at(0)->getQuantum());
    total_quant ++;
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
    if(!handle_signals(SIG_BLOCK)){return  -1;}
    if (threads_counter >= MAX_THREAD_NUM){
        std::cerr << "thread library error: too many threads\n";
        handle_signals(SIG_UNBLOCK);
        return -1;
    }
    if(priority < 0 || priority >= list_size){
        std::cerr << "thread library error: bad priority\n";
        handle_signals(SIG_UNBLOCK);
        return -1;
    }
    auto addr =  (address_t)f;
    for (int i = 0; i < MAX_THREAD_NUM; ++i) {
        if (threads[i] == nullptr){

            threads.at(i) = new thread(quantums_list[priority], i, addr);
            if(threads[i] == nullptr){
                std::cerr << "system error: can't alloc memory\n";
                handle_signals(SIG_UNBLOCK);
                return -1;
            }
            tready.push_back(i);
            threads_counter ++;
            if(!handle_signals(SIG_UNBLOCK)){return -1;}
            return i;
        }
    }
    std::cerr << "thread library error: too many threads\n";
    handle_signals(SIG_UNBLOCK);
    return -1;
}

/*
 * Description: This function changes the priority of the thread with ID tid.
 * If this is the current running thread, the effect should take place only the
 * next time the thread gets scheduled.
 * Return value: On success, return 0. On failure, return -1.
*/
int uthread_change_priority(int tid, int priority){
    if(!handle_signals(SIG_BLOCK)){return -1;}
    if(priority < 0  || priority >= list_size){
        std::cerr <<  "thread library error: wrong priority\n";
        handle_signals(SIG_UNBLOCK);
        return -1;
    }
    if(tid < 0 || tid >= MAX_THREAD_NUM || threads[tid] == nullptr){
        std::cerr <<  "thread library error: wrong tid\n";
        handle_signals(SIG_UNBLOCK);
        return -1;
    }
    threads[tid]->setQuantum(quantums_list[priority]);
    if(!handle_signals(SIG_UNBLOCK)){return -1;}
    return 0;
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
    if(!handle_signals(SIG_BLOCK)){return -1;}
    if(tid >= MAX_THREAD_NUM || threads[tid] == nullptr || tid < 0 ){
        std::cerr <<  "thread library error: the thread dose not exist !\n";
        handle_signals(SIG_UNBLOCK);
        return -1;
    }
    if(tid == 0){
        for (int i = 0; i < MAX_THREAD_NUM; ++i) {
            if(threads[i] != nullptr){
                delete(threads.at(i));
                threads[i] = nullptr;
            }
            threads.clear();
            tready.erase(tready.begin(), tready.end());
        }
        sigemptyset(&sigSet);
        quantums_list = nullptr;
        if(!handle_signals(SIG_UNBLOCK)){return -1;}
        exit(0);
    }
    else{
        threads_counter --;
        int terminated_state = threads[tid]->getState();
        switch(terminated_state){
            case RUN:
            {
                delete(threads.at(tid));
                threads.at(tid) = nullptr;
                round_robin();
                if(!handle_signals(SIG_UNBLOCK)){return -1;}
                return 0;
            }
            case BLOCK:
            {
                delete(threads[tid]);
                threads[tid] = nullptr;
                if(!handle_signals(SIG_UNBLOCK)){return -1;}
                return 0;

            }
            case READY:
            {
                tready.erase(std::remove(tready.begin(), tready.end(), tid), tready.end());
                delete(threads[tid]);
                threads[tid] = nullptr;
                if(!handle_signals(SIG_UNBLOCK)){return -1;}
                return 0;
            }
        }
        threads[tid] = nullptr;
        if(!handle_signals(SIG_UNBLOCK)){return -1;}
        return 0;
    }
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
    if(!handle_signals(SIG_BLOCK)){return -1;}
    if(threads[tid] == nullptr || tid < 0 || tid >= MAX_THREAD_NUM){
        std::cerr <<  "thread library error: no thread like that\n";
        handle_signals(SIG_UNBLOCK);
        return -1;
    }
    if(tid == 0)
    {
        std::cerr <<  "thread library error: can't block the main thread\n";
        handle_signals(SIG_UNBLOCK);
        return -1;
    }
    int blocking_state = threads[tid]->getState();
    if(blocking_state == READY){
        tready.erase(std::remove(tready.begin(), tready.end(), tid), tready.end());
        threads[tid]->setState(BLOCK);
    }
    else if(blocking_state == RUN) {
        threads[tid]->setState(BLOCK);
        handle_signals(SIG_UNBLOCK);
        round_robin(true);
        return 0;

    }
    if(!handle_signals(SIG_UNBLOCK)){return -1;}
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
    if(!handle_signals(SIG_BLOCK)){return -1;}
    if(tid < 0 || tid >= MAX_THREAD_NUM || threads[tid] == nullptr){
        std::cerr <<  "thread library error: no thread like that\n";
        handle_signals(SIG_UNBLOCK);
        return -1;
    }
    int s = threads[tid]->getState();
    if(s == BLOCK){
        tready.push_back(tid);
        threads[tid]->setState(READY);
    }
    if(!handle_signals(SIG_UNBLOCK)){return -1;}
    return 0;
}

/*
 * Description: This function returns the thread ID of the calling thread.
 * Return value: The ID of the calling thread.
*/
int uthread_get_tid(){
    return current_thread;
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
int uthread_get_quantums(int tid) {
    if(!handle_signals(SIG_BLOCK)){return -1;}
    if (tid < 0 || tid >= MAX_THREAD_NUM || threads[tid] == nullptr) {
        std::cerr << "thread library error: the thread dose not exist !\n";
        handle_signals(SIG_UNBLOCK);
        return -1;
    }
    if(!handle_signals(SIG_UNBLOCK)){return -1;}
    return threads[tid]->getCall();
}
