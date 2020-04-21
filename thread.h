//
// Created by galtoledano on 16/04/2020.
//

#ifndef OS2_THREAD_H
#define OS2_THREAD_H


const int READY = 1;

const int BLOCK = 2;

const int RUN = 0;

class thread {
private:
    int quantum;
    int id;
    int call;

public:
    void setQuantum(int quantum);

private:
    int state;
    void(* f)(void);

public:
    thread(int quantum, int id, void (*foo)(void));

    void setState(int state);

    int getQuantum() const;

    int getId() const;

    int getState() const;

    int getCall() const;

    void inc_calls(){call ++;}

};


#endif //OS2_THREAD_H
