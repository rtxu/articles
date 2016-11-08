#include <thread>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <chrono>
#include <ctime>

extern "C" {

void segfault() {
    int *p = 0;
    *p = 0;
}

void segfault_in_another_thread() {
    std::thread t1(segfault);
    t1.detach();    // std::terminate() if no detach when t1 destructs
}

}
