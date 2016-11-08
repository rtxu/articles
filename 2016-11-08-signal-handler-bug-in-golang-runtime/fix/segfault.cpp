#include <thread>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

extern "C" {

static void segvhandler(int signum) {
    abort();
}

static void __attribute__ ((constructor)) sigsetup(void) {
    struct sigaction act;

    memset(&act, 0, sizeof act);
    act.sa_handler = segvhandler;
    sigaction(SIGSEGV, &act, NULL);
}

void segfault() {
    int *p = 0;
    *p = 0;
}

void segfault_in_another_thread() {
    sigsetup();
    std::thread t1(segfault);
    t1.detach();
}

}
