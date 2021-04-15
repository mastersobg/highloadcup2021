#include "error.h"
#include <ostream>

std::ostream &operator<<(std::ostream &os, const ErrorCode &ec) {
    os << (int) ec;
    return os;
}