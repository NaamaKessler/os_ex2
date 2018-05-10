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

    /**
     * Pushes dependent.
     * @param thread
     */
    void pushDependent(Thread *thread);

    /**
     * Pops dependent.
     * @param thread
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

    /**
     * Increase by 1 the counter of num of quantums the thread was running.
     */
    void increaseNumQuantums();

    /**
     * Raise a flag to indicate whether thread was blocked by teminate(),
     * but was not synced to another thread.
     * @param flag
     */
    void setBlockedNoSync(bool flag);

    /**
     * return the flag indicating whether thread was blocked by teminate(),
     * but was not synced to another thread.
     * @return
     */
    bool getBlockedNoSync();

    /**
     * Raise a flag to indicate whether the thread was synced to another thread.
     * @param flag
     */
    void setSynced(bool flag);

    /**
     * Return the flag which indicates whether the thread was synced to another
     * thread.
     * @return
     */
    bool isSynced();

private:
    int _tid, _status, _numQuantums;
    bool _isSynced, _blockedNoSync;
    char _stack[STACK_SIZE];
    queue<Thread*> _dependencyQueue;
    sigjmp_buf _contextBuf;

};

#endif //EX2_THREAD_H