#include "util.h"

void writeIntToString(int64_t n, std::string &s) {
    if (n < 0) {
        s += '-';
        n = -n;
    }

    auto i = s.size();
    for (int64_t limit = 10; n >= limit && limit > 0; limit *= int64_t(10)) {
        i++;
        s += '0';
    }
    s += '0';

    do {
        s[i--] = char(n % 10 + '0');
        n /= 10;
    } while (n != 0);

}

bool equal(double a, double b) noexcept {
    return fabs(a - b) < eps;
}

bool moreOrEqual(double a, double b) noexcept {
    return a > b || equal(a, b);
}
