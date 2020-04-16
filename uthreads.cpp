//
// Created by galtoledano on 16/04/2020.
//

#include "uthreads.h"
#include "thread.h"
#include <iostream>
#include <deque>
#include <vector>
#include <algorithm>
#include <iterator>

void empty(){}
static int threads_counter = 1;
std::deque<thread*> tready;
std::vector<int> quantums_list;
std::vector<thread*> threads(MAX_THREAD_NUM);



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

int uthread_get_tid(){
    // todo: implement
    return 0;
}

int uthread_get_total_quantums(){
    // todo: implement
    return 0;
}

int uthread_get_quantums(int tid){
    // todo: implement
    return 0;
}

void round_robin(){

}