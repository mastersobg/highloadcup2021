#include <iostream>
#include <csignal>
#include <thread>
#include <vector>
#include "log.h"

int main() {

    std::signal(SIGINT, []([[maybe_unused]]int signal) {
        exit(0);
    });

    debugf("Started %d", 1);

    return 0;
}
