/**
 * @file Thread.cpp
 * @brief A thread class.
 *
 */

// ------------------------------ includes ------------------------------
#include "Thread.h"


// ------------------------------- methods ------------------------------



/**
 * @brief Constructor with thread ID.
 * @param tid - thread ID.
 */
Thread::Thread(int tid, void (*f)(void))
{
    this->_tid = tid;
    this->_dependencyQueue =*(new std::vector<Thread*>);
    this->_status = READY;
    this->_sp = (address_t)this->_stack + STACK_SIZE - sizeof(address_t);
    this->_pc = (address_t)f;
}


/**
 * @return Thread ID
 */
int Thread::getId()
{
    return this->_tid;
}

/**
 * Set thread status
 * @param status - READY/RUNNING/BLOCKED
 * @return 0 - success, -1 - failure
 */
int Thread::setStatus(int status)
{
    if (status == READY || status == RUNNING || status == BLOCKED)
    {
        this->_status = status;
        return 0;
    }
    return -1;
}

/**
 * Get thread's state.
 */
int Thread::getStatus()
{
    return this->_status;
}

/**
 *
 * @param thread
 * @return
 */
int Thread::pushDependent(Thread *thread)
{
    this->_dependencyQueue.push_back(thread);
}

/**
 *
 * @param thread
 * @return
 */
Thread* Thread::popDependent(Thread *thread)
{
    Thread* t = nullptr;
    if (!_dependencyQueue.empty())
    {
        t = this->_dependencyQueue.front();
        this->_dependencyQueue.pop_back();

    }
    return t;
}