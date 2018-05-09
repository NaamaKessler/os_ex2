/**
 * @file Thread.cpp
 * @brief A thread class.
 *
 */

// ------------------------------ includes ------------------------------
#include "Thread.h"

#define STACK_SIZE 4096

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
Thread::Thread(int tid, void (*f)(void), int stackSize)
{
    address_t sp, pc;
    this->_blockedNoSync = false;
    this->_isSynced = false;
    this->_tid = tid;
    this->_dependencyQueue =*(new std::queue<Thread*>);
    this->_status = READY;
//    this->_stack = new char[STACK_SIZE];
//    this->_stack = new char[STACK_SIZE];
    this->_numQuantums = 0;
    sp = (address_t)this->_stack + stackSize - sizeof(address_t);
    pc = (address_t)f;
    sigsetjmp(this->_contextBuf, 1);
    (this->_contextBuf->__jmpbuf)[JB_SP] = translate_address(sp);
    (this->_contextBuf->__jmpbuf)[JB_PC] = translate_address(pc);
    sigemptyset(&_contextBuf->__saved_mask);
}

/**
 * Destructor.
 */
Thread::~Thread()
{
//    free(this->_stack);
}

/**
 * Raise a flag to indicate that whether the thread was blocked by teminate(),
 * but was not synced to another thread.
 * @param flag
 */
void Thread::setBlockedNoSync(bool flag)
{
    this->_blockedNoSync = flag;
}

bool Thread::getBlockedNoSync()
{
    return this->_blockedNoSync;
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
//        if (status == RUNNING) {
//            this->_numQuantums++;
//        }
        return 0;
    }
    return -1;
}


queue<Thread*> * Thread::getDependencies()
{
    return &(this->_dependencyQueue);
}

/**
 * get a pointer to the thread's enivronment. (Context Buf)
 */
sigjmp_buf* Thread::getEnvironment(){
    return &(this->_contextBuf);
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
void Thread::pushDependent(Thread *thread)
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

/**
 * Returns the number of quantms the thread with ID tid was in RUNNING state.
 */
int Thread::getNumQuantums()
{
    return this->_numQuantums;
}

/**
* Increase by 1 the counter of num of quantums the thread was running.
*/
void Thread::increaseNumQuantums()
{
//    cerr << "THREAD "<< this->_tid << " QUANTUMS: " << _numQuantums << endl;
    _numQuantums++;
}

/**
 * Raise a flag to indicate whether the thread was synced to another thread.
 * @param flag
 */
void Thread::setSynced(bool flag)
{
    this->_isSynced = flag;
}

bool Thread::isSynced()
{
    return this->_isSynced;
}