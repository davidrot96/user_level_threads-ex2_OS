#ifndef EX2_OS_TREADS_H
#define EX2_OS_TREADS_H

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include "uthreads.h"




typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/**
 * enum for threads state
 */
enum STATS{
    READY,
    RUNNING,
    BLOCK,
    TERMINATE
};

/**
 * enum for mutex state of thread
 */
enum MUTEX{
    LOCK,
    UNLOCK
};

/**
 * class to create thread
 */
class Threads{
private:
    /**
     * _state: state of thread, _mutex_lock: mutex lock of thread
     */
    int _state, _mutex_lock = UNLOCK;

    /**
     * _id: thread id, num_quantum: the num quantums that the thread run
     */
    int _id, num_quantum;

    /**
     * stack for thread
     */
    char _stack[STACK_SIZE];

    /**
     * entry function for thread
     */
    void (*_f)();

public:

    /**
     * constructor for thread
     * @param f entry function
     * @param id thread id
     */
    Threads(void (*f)(void),int id);

    /**
     * default constructor
     */
    Threads();

    /**
     * @return the number of quantums that the thread run
     */
    int get_quantum() const;

    /**
     * @return thread id
     */
    int get_id() const;

    /**
     * @return thread state
     */
    int get_state() const ;

    /**
     * set state for the thread
     */
    void set_state(int state);

    /**
     * increase the num quantums in one
     */
    void inc_quantum();

    /**
     * @return thread nutex_lock state
     */
    int get_mutex_lock() const;

    /**
     * aet mutex_lock state for the thread
     */
    void set_mutex_lock(int m);

    /**
     * sigjmp buffer for thread
     */
    sigjmp_buf _thread{};

    /**
     * translate address
     */
    static address_t translate_address(address_t addr);
};
# endif