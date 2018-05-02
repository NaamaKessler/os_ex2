/**
 * @file uthreads.cpp
 *
 */

// ------------------------------ includes ------------------------------

#include <iostream>
#include <exception>
#include <sys/time.h>
#include <vector>
#include <algorithm>
#include "uthreads.h"
#include "Thread.h"

#define ERR_FUNC_FAIL "thread library error: "
#define ERR_SYS_CALL "system error: "


// ------------------------------- globals ------------------------------

static std::vector<Thread*> buf;
static std::vector<Thread*> readyBuf;
static int numThreads;
static int currentThreadId;

//timer globals:
struct sigaction sa;
static struct itimerval timer;

// ---------- buf;--------------------- methods ------------------------------


int _idValidator(int tid) // copied from uthread_init
{
    // check validity of input
    if (tid < 0 || tid > MAX_THREAD_NUM || !buf[tid]) {
        std::cerr << ERR_FUNC_FAIL << "Invalid input.\n";
        return -1;
    }
    return 0;
}

void schedualer(int sig){

}


/**
 * Initializes a buffer to contain all existing threads.
 */
int initBuffer() {
    try {
        buf = new std::vector(MAX_THREAD_NUM);
    }
    catch (std::bad_alloc& e){
        std::cerr << ERR_SYS_CALL << "Memory allocation failed.\n";
        return -1;
    }
    return 0;

}

/**
 * Sets a virtual timer with the time interval quantum_usecs.
 */
int setTimer(int quantum_usecs) {

    //set timer handler:
    sa.sa_handler = &schedualer;
    if (sigaction(SIGVTALRM, &sa, nullptr) < 0) {
        std::cerr << ERR_SYS_CALL << "sigaction has failed.\n";     //todo: change.
        return -1;
    }

    // Configure the timer to expire after quantum micro secs:
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = quantum_usecs;

    // configure the timer to expire every quantum micro secs after that:
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = quantum_usecs;

    // Start a virtual timer. It counts down whenever this process is executing.
    if (setitimer (ITIMER_VIRTUAL, &timer, nullptr)) {
        std::cerr << ERR_SYS_CALL << "Setting the virtual timer has failed.\n";
        return -1;
    }
}


/*
 * Description: This function initializes the thread library.
 * You may assume that this function is called before any other thread library
 * function, and that it is called exactly once. The input to the function is
 * the length of a quantum in micro-seconds. It is an error to call this
 * function with non-positive quantum_usecs.
 * Return value: On success, return 0. On failure, return -1.
*/
int uthread_init(int quantum_usecs)
{
    if (quantum_usecs <= 0) {
        std::cerr << ERR_FUNC_FAIL << "invalid quantum len was supplied.\n";
        return -1;
    }

    // initialize variables and create an object from the main thread:
    if (initBuffer() < 0) {
        return -1;
    }
    buf[0] = new Thread(0, nullptr);    //todo: how to set entry point to the main thread?
    buf[0]->setStatus(RUNNING);
    numThreads = 1;
    currentThreadId = 0;

    // set timer:
    if (setTimer(quantum_usecs) < 0) {
        return -1;
    }
}

/*
 * Description: This function creates a new thread, whose entry point is the
 * function f with the signature void f(void). The thread is added to the end
 * of the READY threads list. The uthread_spawn function should fail if it
 * would cause the number of concurrent threads to exceed the limit
 * (MAX_THREAD_NUM). Each thread should be allocated with a stack of size
 * STACK_SIZE bytes.
 * Return value: On success, return the ID of the created thread.
 * On failure, return -1.
*/
int uthread_spawn(void (*f)(void))
{
    int tid = -1;
    if (numThreads < MAX_THREAD_NUM)
    {
        //assign id
        for (int i=0; i<MAX_THREAD_NUM; i++)
        {
            if (buf[i] == NULL)
            {
                tid = i;
            }
        }

        //f points to the starting point - pc - of the thread
        Thread* t = new Thread(tid, f);
        readyBuf.push_back(t); // not necessarily at tid - order of ready
        buf[tid] = t; // inserts thread in the minimal open tid, not end of line
        numThreads++;
    }
    return tid;

}


/**
 * Remove thread from the specified buffer by ID.
 * @param buffer
 * @param tid
 */
void removeFromBuf(std::vector<Thread*> buffer, int tid)
{
    for (int idx = 0; idx < buffer.size(); idx++) {
        if (buffer[idx]->getId() == tid) {
            buffer.erase(buffer.begin() + idx);
        }
    }
}

/**
 * Upon termination of a thread, informs all the threads that are synced to it.
 * @param tid
 */
void informDependents(int tid)
{
    Thread* dependent;
    for (int i = 0; i < buf[tid]->getDependentsNum(); i++) {
        dependent = buf[tid]->popDependent();
        dependent->setStatus(READY);
        readyBuf.push_back(dependent);
    }
}

/*
 * Description: This function terminates the thread with ID tid and deletes
 * it from all relevant control structures. All the resources allocated by
 * the library for this thread should be released. If no thread with ID tid
 * exists it is considered an error. Terminating the main thread
 * (tid == 0) will result in the termination of the entire process using
 * exit(0) [after releasing the assigned library memory].
 * Return value: The function returns 0 if the thread was successfully
 * terminated and -1 otherwise. If a thread terminates itself or the main
 * thread is terminated, the function does not return.
*/
int uthread_terminate(int tid)
{
    // check validity of input
    if (tid < 0 || tid > MAX_THREAD_NUM || !buf[tid]) {
        std::cerr << ERR_FUNC_FAIL << "Invalid input.\n";
        return -1;
    }

    // terminated thread != main thread:
    else if (tid) {
        bool callScheduler = false;
        // inform all depending threads:
        informDependents(tid);
        // pop out of ready list:
        if (buf[tid]->getStatus() == READY) {
            removeFromBuf(readyBuf, tid);
        }
        else if (buf[tid]->getStatus() == RUNNING) {
            callScheduler = true;
        }
        // delete thread:
        delete buf[tid];
        buf[tid] = nullptr;
        numThreads--;
        if (callScheduler){
            //call scheduler.
        }
        return 0;
    }
    // terminate the main thread:
    else {
        for (Thread* thread: buf) {
            delete(thread);
        }
        vector<Thread*> dummy_1, dummy_2;
        buf.swap(dummy_1);
        readyBuf.swap(dummy_2);
        exit(0);
    }
}


/*
 * Description: This function blocks the thread with ID tid. The thread may
 * be resumed later using uthread_resume. If no thread with ID tid exists it
 * is considered as an error. In addition, it is an error to try blocking the
 * main thread (tid == 0). If a thread blocks itself, a scheduling decision
 * should be made. Blocking a thread in BLOCKED state has no
 * effect and is not considered an error.
 * Return value: On success, return 0. On failure, return -1.
*/
int uthread_block(int tid)
{
    // check id validity
    if (_idValidator(tid)==-1)
    {
        return -1;
    }

    // remove from ready
    if (buf[tid]->getStatus() == READY)
    {
        for (int i=0; i<currentThreadId; i++)
        {
            if (readyBuf[i]!= NULL && readyBuf[i]->getId()==tid)
            {
                readyBuf[i] = NULL; // is this a good way to delete??
                // can't pop bc it's not necessarily at the top
            }
        }
    }
    buf[tid]->setStatus(BLOCKED);


    return 0;
}




/*
 * Description: This function resumes a blocked thread with ID tid and moves
 * it to the READY state. Resuming a thread in a RUNNING or READY state
 * has no effect and is not considered as an error. If no thread with
 * ID tid exists it is considered an error.
 * Return value: On success, return 0. On failure, return -1.
*/
int uthread_resume(int tid)
{
    return 0;
}


/*
 * Description: This function blocks the RUNNING thread until thread with
 * ID tid will terminate. It is considered an error if no thread with ID tid
 * exists or if the main thread (tid==0) calls this function. Immediately after the
 * RUNNING thread transitions to the BLOCKED state a scheduling decision should be made.
 * Return value: On success, return 0. On failure, return -1.
*/
int uthread_sync(int tid)
{
    return 0;
}


/*
 * Description: This function returns the thread ID of the calling thread.
 * Return value: The ID of the calling thread.
*/
int uthread_get_tid()
{
    return 0;
}


/*
 * Description: This function returns the total number of quantums since
 * the library was initialized, including the current quantum.
 * Right after the call to uthread_init, the value should be 1.
 * Each time a new quantum starts, regardless of the reason, this number
 * should be increased by 1.
 * Return value: The total number of quantums.
*/
int uthread_get_total_quantums()
{
    return 0;
}


/*
 * Description: This function returns the number of quantums the thread with
 * ID tid was in RUNNING state. On the first time a thread runs, the function
 * should return 1. Every add