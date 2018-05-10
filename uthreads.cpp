/**
 * @file uthreads.cpp
 *
 */

// ------------------------------ includes ------------------------------

#include <iostream>
#include <sys/time.h>
#include <vector>
#include <algorithm>
#include <signal.h>
#include <cassert>
#include "uthreads.h"
#include "Thread.h"

#define ERR_FUNC_FAIL "thread library error: "
#define ERR_SYS_CALL "system error: "
#define AFTER_JUMP 2


//todo:
// check makefile

// ------------------------------- globals ------------------------------

static std::vector<Thread*> buf(MAX_THREAD_NUM);
static std::deque<Thread*> readyBuf;
static int numThreads, currentThreadId, totalQuantumNum;
sigset_t blockSet;
bool isReady = true; // state of the currently running thread , before timeHandler is called.

//timer globals:
struct sigaction sa;
static struct itimerval timer;

// -------------------------- inner funcs ------------------------------

// declarations so we can keep up with our funcs
int idValidator(int tid);
void timeHandler(int sig);
void scheduler(int sig);
void contextSwitch(int tid);
int setTimer(int quantum_usecs);
void removeFromBuf(std::deque<Thread*> * buffer, int tid);
void informDependents(int tid);
void mask();
void unMask();
int resetTimer();

// ---------------------------- helper methods --------------------------------


/**
 * Terminates the main thread and frees the allocated memory of the library.
 * @param retVal
 */
void exitLib(int retVal)
{
    sigprocmask(SIG_BLOCK, &blockSet, nullptr);
    for (Thread* thread: buf) {
        if (thread){
            delete(thread);
        }
    }
    vector<Thread*> dummy_1;
    deque<Thread*> dummy_2;
    buf.swap(dummy_1);
    readyBuf.swap(dummy_2);

    exit(retVal);
    }


/**
 * Resets the timer, and updates total quantums and quantums per current
 * thread.
 * @return 0 on success, -1 on failure.
 */
int resetTimer()
{
    buf[currentThreadId]->increaseNumQuantums();
    if (setitimer (ITIMER_VIRTUAL, &timer, nullptr)) {
        std::cerr << ERR_SYS_CALL << "Resetting the virtual timer has failed.\n";
        exitLib(-1);
    }
    totalQuantumNum++;
    return 0;
}

/**
 * check validity of tid.
 */
int idValidator(int tid)
{
    if (tid < 0 || tid > MAX_THREAD_NUM || !buf[tid]) {
        return -1;
    }
    return 0;
}


/**
 * times up -> signal -> time handler is called:
 * calls Scheduler (handles buf & states), that calls contextSwitch (handles env)
 * @param sig
 */
void timeHandler(int sig){
    mask();
    if (isReady){
        scheduler(READY);
    } else {
        scheduler(BLOCKED);
    }
    isReady = true;
    unMask();

}


/**
 * Determine who's running next: moves current thread to READY,
 * pops from ready into RUNNING. Calls context switch.
 * @param state - state to move the current thread to
 */
void scheduler(int state){
    Thread *runningThread;
    int oldID;

    assert (state == READY || state == RUNNING || state == BLOCKED);

    if (readyBuf.size() == 0)
    {
        resetTimer();
        // main thread is running - do nothing
        return;
    } else {
        // move old running thread to readybuf, READY state:
        if (currentThreadId != -1){
            if (buf.at(uthread_get_tid())->getStatus() == RUNNING){
                readyBuf.push_back(buf[uthread_get_tid()]);
            }
            buf[uthread_get_tid()]->setStatus(state);
            oldID = uthread_get_tid();
        }
        else {
            oldID = -1;
        }

        // pop new running thread from ready to running
        runningThread = readyBuf.front();
        readyBuf.pop_front(); // pop just deletes the element
        runningThread->setStatus(RUNNING);
        currentThreadId = runningThread->getId();

        if (oldID != -1) {
            contextSwitch(oldID);
        }
        else {
            resetTimer();
            siglongjmp(*(buf[uthread_get_tid()]->getEnvironment()), AFTER_JUMP);
        }
    }
}

void contextSwitch(int tid){

    // save environment:
    int ret_val = sigsetjmp(*(buf[tid]->getEnvironment()),1);
    if (ret_val == AFTER_JUMP) {
        return;
    }
    resetTimer();
    // load environment:
    siglongjmp(*(buf[uthread_get_tid()]->getEnvironment()),AFTER_JUMP);
}

/**
 * Sets a virtual timer with the time interval quantum_usecs.
 */
int setTimer(int quantum_usecs) {
    //set timer handler:
    sa.sa_handler = &timeHandler;
    if (sigaction(SIGVTALRM, &sa, nullptr) < 0) {
        std::cerr << ERR_SYS_CALL << "sigaction has failed.\n";
        exitLib(-1);
//        return -1;
    }
    // Configure the timer to expire after quantum micro secs:
    timer.it_value.tv_sec = quantum_usecs / 1000000;
    timer.it_value.tv_usec = quantum_usecs % 1000000;

    // configure the timer to expire every quantum micro secs after that:
    timer.it_interval.tv_sec = quantum_usecs / 1000000;
    timer.it_interval.tv_usec = quantum_usecs % 1000000;

    // Start a virtual timer. It counts down whenever this process is executing.
    if (setitimer (ITIMER_VIRTUAL, &timer, nullptr)) {
        std::cerr << ERR_SYS_CALL << "Setting the virtual timer has failed.\n";
//        return -1;
        exitLib(-1);
    }
    return 0;
}

/**
 * Mask the timer signal.
 * @return 0 on success, -1 on failure.
 */
void mask(){
    if (sigprocmask(SIG_BLOCK, &blockSet, nullptr)){
        std::cerr << ERR_SYS_CALL << "Masking failed.\n";
        exitLib(-1);
    }
}

/**
 * Release blocked signals.
 * @return 0 on success, -1 on failure.
 */
void unMask(){
    if (sigprocmask(SIG_UNBLOCK, &blockSet,  nullptr)){
        std::cerr << ERR_SYS_CALL << "Un-masking failed.\n";
        exitLib(-1);
    }
}

/**
 * Remove thread from the specified buffer by ID.
 * @param buffer
 * @param tid
 */
void removeFromBuf(std::deque<Thread*>* buffer, int tid)
{
    for (unsigned int idx = 0; idx < buffer->size(); idx++) {
        if (buffer->at(idx)->getId() == tid) {
            buffer->erase(buffer->begin() + idx);
        }
    }
}

/**
 * Upon termination of a thread, informs all the threads that are synced to it.
 * @param tid
 */
void informDependents(int terminatedId)
{
    Thread* dependent;
    for (int i = 0; i < buf[terminatedId]->getDependentsNum(); i++) {
        dependent = buf[terminatedId]->popDependent();
        if (!(dependent->getBlockedNoSync())){
            dependent->setStatus(READY);
            readyBuf.push_back(dependent);
            dependent->setSynced(false);
        }
    }
}


// ---------------------------- library methods --------------------------------

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
    buf[0] = new Thread(0, nullptr, STACK_SIZE);
    buf[0]->setStatus(RUNNING);
    numThreads = 1;
    currentThreadId = 0;
    totalQuantumNum = 1; // "Right after the call to uthread_init, the value should be 1."

    //init masking-signals buffers:
    if (sigemptyset(&blockSet)) {
        std::cerr << ERR_SYS_CALL << "Signals buffer action has failed.\n";
        exitLib(-1);
    }
    if (sigaddset(&blockSet, SIGVTALRM)){
        std::cerr << ERR_SYS_CALL << "Signals buffer action has failed.\n";
        exitLib(-1);
    }

    // set timer:
    if (setTimer(quantum_usecs) < 0) {
        std::cerr << ERR_SYS_CALL << "Timer initialization failed" << std::endl;
//        return -1;
        exitLib(-1);
    }
    buf[currentThreadId]->increaseNumQuantums();
    return 0;
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
        mask();

        //assign id:
        for (int i=1; i<MAX_THREAD_NUM; i++)
        {
            if (buf[i] == nullptr)
            {
                tid = i;
                break;
            }
        }

        // initialize the new thread and insert to buffers:
        auto t = new Thread(tid, f, STACK_SIZE);
        readyBuf.push_back(t);
        buf[tid] = t;
        numThreads++;
        unMask();
    }

    // error handling:
    if (tid == -1){
        std::cout<< ERR_FUNC_FAIL << "Number of threads exceeds limit.\n";
    }
    return tid;

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
    // check validity of input:
    if (idValidator(tid)) {
        std::cerr << ERR_FUNC_FAIL << "Invalid ID to terminate: ID out of range"
                ".\n";
        return -1;
    }
    mask();
    // terminated thread != main thread:
    if (tid) {
        bool callScheduler = false;
        // inform all depending threads:
        informDependents(tid);
        // pop out of ready list:
        if (buf[tid]->getStatus() == READY) {
            removeFromBuf(&readyBuf, tid);
        }
        // if the thread terminates itself, a scheduling decision has to be made
        else if (buf[tid]->getStatus() == RUNNING) {
            callScheduler = true;
        }
        // delete thread:
        delete buf[tid];
        buf[tid] = nullptr;
        numThreads--;
        if (callScheduler){
            isReady = false;
            currentThreadId = -1;
            timeHandler(BLOCKED);
        }
        unMask();
        return 0;
    }
    // terminate the main thread:
    else {
        exitLib(0);
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
    // check id validity and mask:
    if (tid == 0 || idValidator(tid)==-1){
        std::cerr << ERR_FUNC_FAIL << "Invalid ID to block: ID out of range"
                ".\n";
        return -1;
    }
    mask();
    // remove from ready:
    if (buf[tid]->getStatus() == READY) {
        removeFromBuf(&readyBuf, tid);
    }
    // set state:
    buf[tid]->setStatus(BLOCKED);
    buf[tid]->setBlockedNoSync(true);
    // a thread blocks itself - call scheduler
    if (tid == uthread_get_tid()) {
        scheduler(BLOCKED);
    }
    buf.at(tid)->setBlockedNoSync(true); //raise blocked but not synced flag.

    unMask();
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
    if (idValidator(tid)==-1){
        std::cerr << ERR_FUNC_FAIL << "Invalid ID to resume: ID out of range"
                ".\n";
        return -1;
    }
    mask();
    // make sure thread is not active to begin with:
    if (!(buf[tid]->getStatus() == RUNNING || buf[tid]->getStatus() == READY)){
        if (!buf[tid]->isSynced()) //assure thread is not synced (and therefor shouldn't be resumed)
        {
            buf[tid]->setStatus(READY);
            readyBuf.push_back(buf[tid]);
            buf[tid]->setBlockedNoSync(false);
        }

    }
    unMask();
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
    if (idValidator(tid) || tid == 0)
    {
        std::cerr << ERR_FUNC_FAIL << "Invalid ID to sync: ID out of range.\n";
        return -1;
    }
    if (uthread_get_tid() == tid)
    {
        std::cerr << ERR_FUNC_FAIL << "Invalid ID to sync: Cannot sync itself"
                ".\n";
        return -1;
    }
    mask();
    // block current thread
    buf.at(uthread_get_tid())->setStatus(BLOCKED);
    isReady = false;

    // current thread should wait until tid finishes its job
    buf.at(tid)->pushDependent(buf.at(uthread_get_tid()));
    buf.at(uthread_get_tid())->setSynced(true); //raise synced flag

    // RUNNING thread transitions to the BLOCKED state
    //scheduling decision should be made
    scheduler(BLOCKED);

    unMask();
    return 0;
}


/*
 * Description: This function returns the thread ID of the calling thread.
 * Return value: The ID of the calling thread.
*/
int uthread_get_tid()
{
    return currentThreadId;
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
    return totalQuantumNum;
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
    if (idValidator(tid)){
        std::cerr << ERR_FUNC_FAIL << "Thread doesn't exist.\n";
        return -1;
    }
    return buf[tid]->getNumQuantums();
}