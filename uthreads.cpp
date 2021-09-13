#include <iostream>
#include "uthreads.h"
#include "Threads.h"
#include <queue>
#include <map>
#include <list>
#include <signal.h>
#include <sys/time.h>

using namespace std;

/*********************** defines *******************/

#define ERROR -1
#define SUCCESS 0
#define MICRO_SEC_FOR_SEC 1000000
#define LIBRARY_ERROR_MESSAGE "thread library error: "
#define SYSTEM_ERROR_MESSAGE "system error: "

/*********************** declarations *******************/

void rm_mutex();
void delete_threads();
void schedule(int r);
int set_timer();
typedef struct{int state = UNLOCK;int id;}mutex_T;


/*********************** global variables *******************/

priority_queue <int, vector<int>, greater<int>> id_treads;
struct sigaction sa = {};
struct itimerval timer;
int quantumUsecs;
int num_treads = 0,count_id = 1,run_ID = 0,total_quantums = 1;
static map <int, Threads*> map_treads;
list <int> ready_list;
list <int> mutex_block_list;
mutex_T mutex_uthreads;
sigset_t set;
bool thread_terminate = false;


/*********************** libaray functions  *******************/

int uthread_init(int quantum_usecs){
    if(quantum_usecs < 0){
        cerr << LIBRARY_ERROR_MESSAGE << "invalid quantum\n";
        return ERROR;
    }
    sigemptyset(&set);
    sigaddset(&set, SIGVTALRM);
    quantumUsecs = quantum_usecs;
    map_treads[0] = new Threads();
    num_treads++;
    set_timer();
    return SUCCESS;
}

int uthread_spawn(void (*f)(void)){
    sigprocmask(SIG_SETMASK, &set, nullptr);
    int id;
    if(num_treads >= MAX_THREAD_NUM){
        cerr << LIBRARY_ERROR_MESSAGE << "to many threads in the system\n";
        return ERROR;
    }
    num_treads++; // need decrease when the tread is terminate
    if(id_treads.empty()){ // we need a new id because we do nut have more
        id = count_id;
        count_id++;
    } else{ // we have a smaller id
        id = id_treads.top();
        id_treads.pop();
    }
    map_treads[id] = new Threads(f,id);
    ready_list.push_back(id);
    sigprocmask(SIG_UNBLOCK, &set, nullptr);
    return id;
}

int uthread_terminate(int tid){
    auto i = map_treads.find(tid);
    if(i == map_treads.end()){
        cerr << LIBRARY_ERROR_MESSAGE << "the tread with tid not found\n";
        return ERROR;
    }
    sigprocmask(SIG_SETMASK, &set, nullptr);
    num_treads--;
    if(mutex_uthreads.state == LOCK && mutex_uthreads.id == i->second->get_id()){
        mutex_uthreads.state = UNLOCK;
        rm_mutex();
    }
    if(i->second->get_state() == READY && i->second->get_mutex_lock() == UNLOCK) {
        ready_list.remove(tid);

    } else if(tid == 0){  //we need to check if we in running state
        delete_threads();
        exit(0);
    }
    if(i->second->get_state() == RUNNING){
        i->second->set_state(TERMINATE);
        thread_terminate = true;
        sigprocmask(SIG_UNBLOCK, &set, nullptr);
        schedule(0);
    }
    delete map_treads[tid];
    map_treads.erase(tid);
    id_treads.push(tid);
    sigprocmask(SIG_UNBLOCK, &set, nullptr);
    return SUCCESS;
}

int uthread_block(int tid){
    auto i = map_treads.find(tid);
    if(tid == 0 || i == map_treads.end()){
        cerr << LIBRARY_ERROR_MESSAGE << "can not block tread 0 || the tread with tid not found\n";
        return ERROR;
    }
    sigprocmask(SIG_SETMASK, &set, nullptr);
    if(run_ID == tid){ // we want to block the running tread
        i->second->set_state(BLOCK);
        schedule(tid);
    }
    else if(i->second->get_state() == READY && i->second->get_mutex_lock() == UNLOCK) {
        ready_list.remove(tid);
    }
    i->second->set_state(BLOCK);
    sigprocmask(SIG_UNBLOCK, &set, nullptr);
    return SUCCESS;
}

int uthread_resume(int tid){
    auto i = map_treads.find(tid);
    if(i == map_treads.end()){
        cerr << LIBRARY_ERROR_MESSAGE << "the tread with tid not found\n";
        return ERROR;
    }
    sigprocmask(SIG_SETMASK, &set, nullptr);
    if(i->second->get_state() == BLOCK){
        if(i->second->get_mutex_lock() == UNLOCK){
            ready_list.push_back(tid);
        }
        i->second->set_state(READY);
    }
    sigprocmask(SIG_UNBLOCK, &set, nullptr);
    return SUCCESS;
}

int uthread_mutex_lock(){
    if(mutex_uthreads.state == LOCK && mutex_uthreads.id == run_ID){
        cerr << LIBRARY_ERROR_MESSAGE << "can not lock the mutex twice\n";
        return ERROR;
    }
    sigprocmask(SIG_SETMASK, &set, nullptr);
    if(mutex_uthreads.state == UNLOCK){
        mutex_uthreads.state = LOCK;
        mutex_uthreads.id = run_ID;
        return SUCCESS;
    }
    while (mutex_uthreads.state == LOCK) {
        mutex_block_list.push_back(run_ID);
        map_treads[run_ID]->set_mutex_lock(LOCK);
        schedule(run_ID);
    }
    mutex_uthreads.state = LOCK;
    mutex_uthreads.id = run_ID;
    sigprocmask(SIG_UNBLOCK, &set, nullptr);
    return SUCCESS;
}

int uthread_mutex_unlock(){
    if(mutex_uthreads.state == UNLOCK || (mutex_uthreads.state == LOCK && mutex_uthreads.id != run_ID)){
        cerr << LIBRARY_ERROR_MESSAGE << "can not release the mutex twice\n";
        return ERROR;
    }
    sigprocmask(SIG_SETMASK, &set, nullptr);
    mutex_uthreads.state = UNLOCK;
    mutex_uthreads.id = -1;
    rm_mutex();
    sigprocmask(SIG_UNBLOCK, &set, nullptr);
    return SUCCESS;

}

int uthread_get_tid(){
    return run_ID;
}

int uthread_get_total_quantums(){
    return total_quantums;
}

int uthread_get_quantums(int tid){
    sigprocmask(SIG_SETMASK, &set, nullptr);
    auto i = map_treads.find(tid);
    if(i == map_treads.end()){
        cerr << LIBRARY_ERROR_MESSAGE << "the tread with tid not found\n";
        return ERROR;
    }
    int quantum_of_tid = map_treads[tid]->get_quantum();
    sigprocmask(SIG_UNBLOCK, &set, nullptr);
    return quantum_of_tid;
}

/*********************** helper functions *******************/

/*
 * find the next thread that should be released from the mutex block after the mutex released
 * */
void rm_mutex(){
    int flug = true;
    for(int i : mutex_block_list){
        if(map_treads[i]->get_state() == READY){
            map_treads[i]->set_mutex_lock(UNLOCK);
            ready_list.push_back(i);
            mutex_block_list.remove(i);
            flug = false;
            break;
        }
    }
    if(flug && !mutex_block_list.empty()){
        int i = mutex_block_list.front();
        mutex_block_list.pop_front();
        map_treads[i]->set_mutex_lock(UNLOCK);
    }
}

/*
 * Schedule and manage the threads replacement
 */
void schedule(int r){
    sigprocmask(SIG_SETMASK, &set, nullptr);
    total_quantums ++;
    if(ready_list.empty()){
        map_treads[run_ID]->inc_quantum();
        set_timer();
        sigprocmask(SIG_UNBLOCK, &set, nullptr);
        return;
    }
    int save_env_thread = sigsetjmp(map_treads[run_ID]->_thread, 1);
    if(save_env_thread != 0) {
        sigprocmask(SIG_UNBLOCK, &set, nullptr);
        return;
    }
    int current_id = ready_list.front();
    ready_list.pop_front();
    if(map_treads[run_ID]->get_state() == RUNNING){
        map_treads[run_ID]->set_state(READY);
        if (map_treads[run_ID]->get_mutex_lock() == UNLOCK){
            ready_list.push_back(run_ID);
        }
    }
    if(thread_terminate){
        thread_terminate = false;
        delete map_treads[run_ID];
        map_treads.erase(run_ID);
        id_treads.push(run_ID);

    }
    run_ID = current_id;
    map_treads[run_ID]->inc_quantum(); // note when do we need to increse the counter
    map_treads[run_ID]->set_state(RUNNING);
    sigprocmask(SIG_UNBLOCK, &set, nullptr);
    set_timer();
    siglongjmp(map_treads[run_ID]->_thread,1);
}

/*
 * delete all the allocations of threads
 */
void delete_threads(){
    for(auto i : map_treads){
        delete i.second;
    }
}

/*
 * set the timer of the signals
 */
int set_timer(){
    sa.sa_handler = &schedule;
    if (sigaction(SIGVTALRM, &sa,nullptr) < 0) {
        cerr << SYSTEM_ERROR_MESSAGE << "sigaction error\n";
        delete_threads();
        exit(EXIT_FAILURE);
    }
    timer.it_value.tv_sec = quantumUsecs / MICRO_SEC_FOR_SEC;		// first time interval, seconds part
    timer.it_value.tv_usec = quantumUsecs % MICRO_SEC_FOR_SEC;		// first time interval, microseconds part

    timer.it_interval.tv_sec = quantumUsecs / MICRO_SEC_FOR_SEC;	// following time intervals, seconds part
    timer.it_interval.tv_usec = quantumUsecs % MICRO_SEC_FOR_SEC;	// following time intervals, microseconds part

    // Start a virtual timer. It counts down whenever this process is executing.
    if (setitimer (ITIMER_VIRTUAL, &timer, nullptr) < 0) {
        cerr << SYSTEM_ERROR_MESSAGE << "timer error\n";
        delete_threads();
        exit(EXIT_FAILURE);
    }
    return SUCCESS;
}