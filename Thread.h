/**
 * @file Thread.h
 * @brief A thread class.
 *
 */

// ------------------------------ includes ------------------------------

#ifndef EX2_THREAD_H
#define EX2_THREAD_H
#define STACK_SIZE 4096

#include <iostream>
#include <queue>
#include <csetjmp>

// status:
#define READY 1
#define RUNNING 2
#define BLOCKED 3



// ------------------------------- methods ------------------------------
using namespace std;
typedef unsigned long address_t;


class Thread
{
public:
    /**
     * @brief Constructor with thread ID.
     * @param tid - thread ID.
     */
    Thread(int tid, void (*f)(void));

    /**
     * @return Thread ID
     */
    int getId();

    /**
     * Set thread status
     * @param status - READY/RUNNING/BLOCKED
     * @return 0 - success, -1 - failure
     */
    int setStatus(int status);

    /**
     *
     * @param thread
     * @return
     */
    int pushDependent(Thread *thread);

    /**
     *
     * @param thread
     * @return
     */
    Thread* popDependent(Thread *thread);

private:
    int _tid;
    int _status;
    char _stack[STACK_SIZE];
    vector<Thread*> _dependencyQueue;
    address_t _sp, _pc;
    sigjmp_buf _contextBuf;

};


#endif //EX2_THREAD_H
