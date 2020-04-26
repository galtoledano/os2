//
// Created by galtoledano on 16/04/2020.
//

#ifndef OS2_THREAD_H
#define OS2_THREAD_H


#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#define SECOND 1000000
//#define STACK_SIZE 16384

typedef unsigned long address_t;

#define JB_SP 6
#define JB_PC 7

const int READY = 1;

const int BLOCK = 2;

const int RUN = 0;

class thread {
private:
    int quantum;
    int id;
    int call;
    int state;
    void(* f)(void);
    sigjmp_buf env;
    address_t sp, pc;


public:
    thread(int quantum, int id, void (*foo)(void));

    void setState(int state);

    void setQuantum(int quantum);

    int getQuantum() const;

    int getId() const;

    int getState() const;

    int getCall() const;

    void inc_calls(){call ++;}

    sigjmp_buf &getEnv();


};


#endif //OS2_THREAD_H
