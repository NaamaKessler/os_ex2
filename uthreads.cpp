/**
 * @file uthreads.cpp
 *
 */

// ------------------------------ includes ------------------------------

#include <iostream>
#include <exception>
#include <vector>
#include "uthreads.h"
#include "Thread.h"


// ------------------------------- globals ------------------------------

//todo: How to initialize inside init() but as globals?
//Thread** buf[MAX_THREAD_NUM] = {};
//static std::vector<Thread> readyBuf;
//static int numThreads = 1;
//static int currentThread = 0;
//static int quant;

std::vector<Thread*> buf;
static std::vector<Thread*> readyBuf;
int quant;
static int numThreads;
static int currentThreadId;

// ---------- buf;--------------------- methods ------------------------------




std::vector<Thread*> initBuffer() {
    static std::vector<Thread*> buffer(MAX_THREAD_NUM);
    return buffer;

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
    buf = initBuffer();
    quant = quantum_usecs;
    numThreads = 1;
    currentThreadId = 0;
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
    if (numThreads == MAX_THREAD_NUM)
    {
        //
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
    if (tid < 0 || tid > MAX_THREAD_NUM || buf[tid]==nullptr) {
        return -1;
    }
    /// terminate main thread
    else if (!tid) {

    }
    else {
        // inform all depending
        for (Thread* dependent : buf[tid]->getDepndencies()){  //todo: add way to get dependencies
            //move to ready list
            dependent->setStatus(READY);
            readyBuf.push_back(dependent);
        }
    }
    return 0;
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
 * should return 1. Every additional quantum that the thread starts should
 * increase this value by 1 (so if the thread with ID tid is in RUNNING state
 * when this function is called, include also the current quantum). If no
 * thread with ID tid exists it is considered an error.
 * Return value: On success, return the number of quantums of the thread with ID tid.
 * 			     On failure, return -1.
*/
int uthread_get_quantums(int tid)
{
    return 0;
}