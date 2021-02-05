#include <iostream>
#include <csignal>
#include <thread>
#include <vector>

void exec() {
    constexpr u_int32_t size = 1000'000;
    for (;;) {
        int *p = new int[size];
        for (u_int32_t i = 0; i < size; i++) {
            p[i] = std::rand();
        }

        int64_t sum{0};

        for (u_int32_t i = 0; i < size; i++) {
            sum += p[i];
        }

        std::cout << sum << std::endl;


        delete[]p;
    }
}

int main() {

    std::signal(SIGINT, []([[maybe_unused]]int signal) {
        exit(0);
    });

    std::vector<std::thread> threads;
    for (size_t i = 0; i < 10; i++) {
        threads.emplace_back(exec);
    }

    for (auto &t : threads) {
        t.join();
    }
    return 0;
}
