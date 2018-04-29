/**
 * @file Thread.h
 * @brief A thread class.
 *
 */

// todo:
/*
 * _status - 1 for ready?
 *  - dependency queue
*/

// ------------------------------ includes ------------------------------

#ifndef EX2_THREAD_H
#define EX2_THREAD_H
#define STACK_SIZE 4096
#include <stdio.h>
// ------------------------------- methods ------------------------------
using namespace std;

class Thread
{
public:
    /**
     * @brief Constructor with thread ID.
     * @param tid - thread ID.
     */
    Thread(int tid);

    /**
     * @return Thread ID
     */
    int getId() const;

private:
    int _tid; //thread ID
    int _status; // todo
    char _stack[STACK_SIZE];
    queue _dependencyQueue;

};


#endif //EX2_THREAD_H
