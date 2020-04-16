//
// Created by galtoledano on 16/04/2020.
//

#ifndef OS2_THREAD_H
#define OS2_THREAD_H


class thread {
private:
    int quantum;
    int id;
    int state;

public:
    thread(int quantum, int id);

    void setState(int state);

    int getQuantum() const;

    int getId() const;

    int getState() const;
//    ~thread() = default;

};


#endif //OS2_THREAD_H
