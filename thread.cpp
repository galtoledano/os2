//
// Created by galtoledano on 16/04/2020.
//

#include "thread.h"
thread::thread(int quantum, int id) : quantum(quantum), id(id)
{
    this->state = 1;
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

