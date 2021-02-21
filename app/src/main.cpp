#include <iostream>
#include <csignal>
#include <thread>
#include <vector>
#include "log.h"
#include <rapidjson/document.h>

int main() {

    std::signal(SIGINT, []([[maybe_unused]]int signal) {
        exit(0);
    });

    debugf("Started %d", 1);
    rapidjson::Document d;
    char s[] = "{\"a\":1}";
    d.ParseInsitu(s);
    auto val = d["a"].GetInt();
    debugf("value: %d", val);

    return 0;
}
