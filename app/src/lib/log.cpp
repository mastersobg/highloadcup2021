#include "log.h"
#include <mutex>
#include <iostream>

LogWriter Log::debug() {
    return LogWriter(*this, "DEBUG ");
}

LogWriter Log::info() {
    return LogWriter(*this, "INFO  ");
}

LogWriter Log::error() {
    return LogWriter(*this, "ERROR ");
}

LogWriter Log::warn() {
    return LogWriter(*this, "WARN  ");
}

void Log::log(const std::string &s) {
    std::scoped_lock lock(mu_);
    std::cout << s << std::flush;
}

LogWriter::LogWriter(Log &log, const std::string &logLevel) : log_{log} {
    stream << logLevel;
}

LogWriter::~LogWriter() {
    stream << std::endl;
    log_.log(stream.str());
}

LogWriter &LogWriter::operator<<(std::ostream &(*f)(std::ostream &)) {
    stream << f;
    return *this;
}
