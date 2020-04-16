//
// Created by galtoledano on 16/04/2020.
//

#include "uthreads.h"

int uthread_init(int *quantum_usecs, int size);


int uthread_spawn(void (*f)(void), int priority);


int uthread_change_priority(int tid, int priority);


int uthread_terminate(int tid);



int uthread_block(int tid);



int uthread_resume(int tid);


int uthread_get_tid();


int uthread_get_total_quantums();


int uthread_get_quantums(int tid);
