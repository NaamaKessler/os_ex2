/**
 * @file Thread.h
 * @brief A thread class.
 *
 */

// ------------------------------ includes ------------------------------

#ifndef EX2_THREAD_H
#define EX2_THREAD_H

#include <iostream>
#include <queue>
#include <csetjmp>
#include <signal.h>

// status:
#define READY 1
#define RUNNING 2
#define BLOCKED 3

//#define STACK_SIZE 4096

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
    void pushDependent(Thread *thread);

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

    void increaseNumQuantums();

    void setBlockedNoSync(bool flag);

    bool getBlockedNoSync();

    void setSynced(bool flag);

    bool isSynced();

private:
    int _tid;
    bool _isSynced, _blockedNoSync;
    int _status, _numQuantums;
    char* _stack;
    queue<Thread*> _dependencyQueue;
    sigjmp_buf _contextBuf;

};


#endif //EX2_THREAD_H