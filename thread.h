//
// Created by galtoledano on 16/04/2020.
//

#ifndef OS2_THREAD_H
#define OS2_THREAD_H


class thread {
    int quantum;
    int id;
    int state;

    void new thread(int quantum,int id);
};


#endif //OS2_THREAD_H
