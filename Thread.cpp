/**
 * @file Thread.cpp
 * @brief A thread class.
 *
 */

// ------------------------------ includes ------------------------------
#include "Thread.h"

// from demo:
#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
            "rol    $0x11,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}

#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
		"rol    $0x9,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}
#endif

// ------------------------------- methods ------------------------------



/**
 * @brief Constructor with thread ID.
 * @param tid - thread ID.
 */
Thread::Thread(int tid, void (*f)(void))
{
    this->_tid = tid;
    this->_dependencyQueue =*(new std::queue<Thread*>);
    this->_status = READY;
    this->_sp = (address_t)this->_stack + STACK_SIZE - sizeof(address_t);
    this->_pc = (address_t)f;
    (this->_contextBuf->__jmpbuf)[JB_SP] = translate_address(_sp);
    (this->_contextBuf->__jmpbuf)[JB_PC] = translate_address(_pc);
    sigemptyset(&_contextBuf->__saved_mask);
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


queue<Thread*> Thread::getDependencies()
{
    return this->_dependencyQueue;
}

/**
 * get a pointer to the thread's enivronment. (Context Buf)
 */
sigjmp_buf* Thread::getEnvironment(){
    return this->&_contextBuf;
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
    this->_dependencyQueue.push(thread);
}

/**
 *
 * @param thread
 * @return
 */
Thread* Thread::popDependent() //todo: I removed the argument since it wasn't being used
{
    Thread* t = nullptr;
    if (!_dependencyQueue.empty())
    {
        t = this->_dependencyQueue.front();
        _dependencyQueue.pop();

    }
    return t;
}

/**
 * Return the number of threads that are synced to this thread.
 * @return
 */
int Thread::getDependentsNum()
{
    return this->_dependencyQueue.size();
}