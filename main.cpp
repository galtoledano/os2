/**********************************************
 * Test 5: sync/block/resume
 *
 **********************************************/

#include <cstdio>
#include "uthreads.h"
#include <stdio.h>
#include <iostream>


#define GRN "\e[32m"
#define RED "\x1B[31m"
#define RESET "\x1B[0m"

#define NUM_THREADS 4
#define RUN 0
#define DONE 1
char thread_status[NUM_THREADS];


void halt()
{
    while (true)
    {}
}

int next_thread()
{
    return (uthread_get_tid() + 1) % NUM_THREADS;
}

void thread()
{
    //uthread_sync(next_thread());

    //uthread_sync(next_thread());

    std::cout << "in function ! " << std::endl;

    uthread_block(uthread_get_tid());

    for (int i = 0; i < 50; i++)
    {
        uthread_resume(next_thread());
    }

    thread_status[uthread_get_tid()] = DONE;

    halt();
}

bool all_done()
{
//    std::cout << "in all_done" << std::endl;
    bool res = true;
    for (int i = 1; i < NUM_THREADS; i++)
    {
        res = res && (thread_status[i] == DONE);
//        std::cout << res << std::endl;
    }
    return res;
}

int main()
{
    printf(GRN "Test 5:    " RESET);
    fflush(stdout);

    int q[2] = {10, 20};
    uthread_init(q, 2);
    uthread_spawn(thread, 0);
    uthread_spawn(thread, 0);
    uthread_spawn(thread, 1);

    for (int i = 0; i < NUM_THREADS; i++)
    {
        std::cout << "in for" << std::endl;
        thread_status[i] = RUN;
    }

    while (!all_done())
    {
//        std::cout << "in while" << std::endl;
        uthread_resume(1);
    }

    printf(GRN "SUCCESS\n" RESET);
    uthread_terminate(0);

}