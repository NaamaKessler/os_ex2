/**
 * @file Thread.h
 * @brief A thread class.
 *
 */

// ------------------------------ includes ------------------------------

#ifndef EX2_THREAD_H
#define EX2_THREAD_H
#define STACK_SIZE 4096 // maybe should be changed - it's defined in uthreads

#include <iostream>
#include <queue>
#include <csetjmp>
#include <signal.h>

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
    Thread(int tid, void (*f)(void), int stackSize);

    /**
     * Destructor.
     */
    ~Thread();

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
     * Get thread's state.
     */
    int getStatus();

    /**
    * get a pointer to the thread's enivronment. (Context Buf)
    */
    sigjmp_buf* getEnvironment();

    queue<Thread*> * getDependencies();

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
    Thread* popDependent();

    /**
     * Return the number of threads that are synced to this thread.
     * @return
     */
    int getDependentsNum();

    /**
     * Returns the number of quantms the thread with ID tid was in RUNNING state.
     */
    int getNumQuantums();

private:
    int _tid;
    int _status;
    char _stack[STACK_SIZE];
    queue<Thread*> _dependencyQueue;
    address_t _sp, _pc;
    sigjmp_buf _contextBuf;
    int _numQuantums;

};


#endif //EX2_THREAD_H
