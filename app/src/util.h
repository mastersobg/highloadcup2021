#ifndef HIGHLOADCUP2021_UTIL_H
#define HIGHLOADCUP2021_UTIL_H

#include <string>
#include <chrono>


void writeIntToString(int64_t n, std::string &s);

template<class T>
class Measure {
    std::chrono::steady_clock::time_point start_;
public:
    Measure() : start_{std::chrono::steady_clock::now()} {}

    int64_t getInt64() {
        return std::chrono::duration_cast<T>(std::chrono::steady_clock::now() - start_).count();
    }

    int32_t getInt32() {
        return (int32_t) getInt64();
    }

    T getDuration() {
        return std::chrono::duration_cast<T>(std::chrono::steady_clock::now() - start_);
    }

};

#endif //HIGHLOADCUP2021_UTIL_H
