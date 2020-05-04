//
// Created by galtoledano on 16/04/2020.
//

#ifndef OS2_THREAD_H
#define OS2_THREAD_H

#include <stdio.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>



#define SECOND 1000000

#define JB_SP 6

#define JB_PC 7

#define STACK_SIZE 4096 /* stack size per thread (in bytes) */

typedef unsigned long address_t;

const int READY = 1;

const int BLOCK = 2;

const int RUN = 0;


class thread {
private:
    int quantum;
    int id;
    int call;
    int state;
    char* stack;
    sigjmp_buf env;
    address_t pc;
    address_t sp;

public:
    thread(int quantum, int id, address_t foo);

    ~thread(){delete[] stack;}

    void setState(int state);

    void setQuantum(int quantum);

    int getQuantum() const;

    int getId() const;

    int getState() const;

    int getCall() const;

    void inc_calls(){call ++;}

    sigjmp_buf *getEnv();

};


#endif //OS2_THREAD_