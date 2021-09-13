#include "Threads.h"



Threads::Threads(void (*f)(void), int id): _state(READY), _id(id), num_quantum(0), _f(f), _thread() {
    address_t sp, pc;

    sp = (address_t)_stack + STACK_SIZE - sizeof(address_t);
    pc = (address_t)_f;
    sigsetjmp(_thread, 0);
    (_thread->__jmpbuf)[JB_SP] = translate_address(sp);
    (_thread->__jmpbuf)[JB_PC] = translate_address(pc);
}

Threads::Threads(): _state(RUNNING), _id(0), num_quantum(1), _f(nullptr), _thread() {}

int Threads::get_id() const
{
    return _id;
}

int Threads::get_quantum() const
{
    return num_quantum;
}

void Threads::inc_quantum()
{
    num_quantum++;
}

address_t Threads::translate_address(address_t addr) {
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
                 "rol    $0x11,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}

int Threads::get_state() const {
    return _state;
}

void Threads::set_state(int s) {
    _state = s;
}

int Threads::get_mutex_lock() const {
    return _mutex_lock;
}

void Threads::set_mutex_lock(int m) {
    _mutex_lock = m;
}
