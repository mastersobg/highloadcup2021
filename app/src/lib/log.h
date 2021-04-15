#ifndef HIGHLOADCUP2021_LOG_H
#define HIGHLOADCUP2021_LOG_H

#include <mutex>
#include <string>
#include <sstream>

class LogWriter;

class Log {
    std::mutex mu_;

    void log(const std::string &s);

public:
    LogWriter debug();

    LogWriter info();

    LogWriter error();

    LogWriter warn();

    friend class LogWriter;
};

class LogWriter {
    Log &log_;
    std::ostringstream stream;
public:
    LogWriter(Log &log, const std::string &logLevel);

    LogWriter(LogWriter &&other) = default;

    ~LogWriter();

    template<class T>
    LogWriter &operator<<(T t) {
        stream << t;
        return *this;
    }

    LogWriter &operator<<(std::ostream &(*f)(std::ostream &o));

    LogWriter &precision(int precision) {
        stream << std::fixed;
        stream.precision(precision);
        return *this;
    }
};

#endif //HIGHLOADCUP2021_LOG_H
