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

void f1() {
    int tid = uthread_get_tid();
    std::cout << "f1 is runnint - tid is: " << tid << "\n";
    sleep(1);
}

void f2() {
    int tid = uthread_get_tid();
    std::cout << "f2 is runnint - tid is: " << tid << "\n";
    sleep(1);
}

void f3() {
    int tid = uthread_get_tid();
    std::cout << "f3 is runnint - tid is: " << tid << "\n";
    sleep(1);
}

int main() {
    if (uthread_init(QUANTUM_USECS) == -1) {
        std::cerr << "uthread_init failed" << std::endl;
        return 1;
    }
    int f1_tid = uthread_spawn(f1);
    int f2_tid = uthread_spawn(f2);
    int f3_tid = uthread_spawn(f3);

    std::cout << "f1_tid is: " << f1_tid << std::endl;
    std::cout << "f2_tid is: " << f2_tid << std::endl;
    std::cout << "f3_tid is: " << f3_tid << std::endl;

    // int tids[NUM_THREADS];
    // for (int i = 0; i < NUM_THREADS; ++i) {
    //     tids[i] = uthread_spawn(thread_function);
    //     if (tids[i] == -1) {
    //         std::cerr << "uthread_spawn failed for thread " << i << std::endl;
    //         return 1;
    //     }
    // }
    // // tids[NUM_THREADS] = uthread_spawn(printer);
    // for (int i = 0; i < NUM_THREADS; ++i) {
    //     std::cout << tids[i] << std::endl;
    // }

    // // Main thread also does some work
    // for (int i = 0; i < 3; ++i) {
    //     std::cout << "Main thread is running. Quantum " << uthread_get_quantums(0) << std::endl;
    //     sleep(1);
    // }
    // sleep(5);
    

    // // Terminate all threads
    // for (int i = 0; i < NUM_THREADS; ++i) {
    //     uthread_terminate(tids[i]);
    // }
    // uthread_terminate(0); // Terminate main thread.

    // std::cout << "Total quantums: " << uthread_get_total_quantums() << std::endl;

    return 0;
}