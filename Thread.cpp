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
}

/**
 * @return Thread ID
 */
int getId()
{
    return _tid;
}
