#include "uthreads.h"
#include <iostream>
#include <unistd.h> // For sleep()

#define NUM_THREADS 3
#define QUANTUM_USECS 5000 // 0.5 seconds

void thread_function(void) {
    std::cout << "I'm runnint!\n"; 
    int tid = uthread_get_tid();
    for (int i = 0; i < 3; ++i) {
        std::cout << "Thread " << tid << " is running. Quantum " << uthread_get_quantums(tid) << std::endl;
        sleep(1); // Simulate some work
    }
}


int main() {
    if (uthread_init(QUANTUM_USECS) == -1) {
        std::cerr << "uthread_init failed" << std::endl;
        return 1;
    }

    int tids[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; ++i) {
        tids[i] = uthread_spawn(thread_function);
        if (tids[i] == -1) {
            std::cerr << "uthread_spawn failed for thread " << i << std::endl;
            return 1;
        }
    }
    // tids[NUM_THREADS] = uthread_spawn(printer);
    for (int i = 0; i < NUM_THREADS; ++i) {
        std::cout << tids[i] << std::endl;
    }

    // Main thread also does some work
    for (int i = 0; i < 3; ++i) {
        std::cout << "Main thread is running. Quantum " << uthread_get_quantums(0) << std::endl;
        sleep(1);
    }
    sleep(5);
    

    // Terminate all threads
    for (int i = 0; i < NUM_THREADS; ++i) {
        uthread_terminate(tids[i]);
    }
    uthread_terminate(0); // Terminate main thread.

    std::cout << "Total quantums: " << uthread_get_total_quantums() << std::endl;

    return 0;
}