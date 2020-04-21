//
// Created by galtoledano on 16/04/2020.
//

#include "thread.h"
thread::thread(int quantum, int id, void (*foo)(void)) : quantum(quantum), id(id)
{
    //todo allloc memory
    this->f = foo;
    this->state = READY;
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

