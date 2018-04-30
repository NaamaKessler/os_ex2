/**
 * @file Thread.cpp
 * @brief A thread class.
 *
 */

// ------------------------------ includes ------------------------------
#include "Thread.h"

// ------------------------------- methods ------------------------------
using namespace Thread;

/**
 * @brief Constructor with thread ID.
 * @param tid - thread ID.
 */
Thread(int tid, void (*f)(void))
{
    this->_tid = tid;
    this->_dependencyQueue = queue<*thread> q;
}

/**
 * @return Thread ID
 */
int getId()
{
    return _tid;
}

/**
 * Set thread status
 * @param status - READY/RUNNING/BLOCKED
 * @return 0 - success, -1 - failure
 */
int setStatus(int status)
{
    if (status == READY || status == RUNNING || status == BLOCKED)
    {
        this->_status = status;
        return 0;
    }
    return -1;
}

/**
 *
 * @param thread
 * @return
 */
int pushDependent(Thread *thread)
{
    this->_dependencyQueue.push(thread);
}

/**
 *
 * @param thread
 * @return
 */
thread* popDependent(Thread *thread)
{
    return this->_dependencyQueue.pop();
}