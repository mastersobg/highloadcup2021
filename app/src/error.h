#ifndef HIGHLOADCUP2021_ERROR_H
#define HIGHLOADCUP2021_ERROR_H

#include <variant>

enum class ErrorCode : int {
    kNoErr = 0,
    kErrCurl = 1,
    kMaxApiRequestsQueueSizeExceeded = 2,
    kUnknownRequestType = 3,
    kIssueLicenceError = 4,
    kNoAvailableLicense = 5,
    kUnexpectedDigResponse = 6,
    kTreasuriesLeftInconsistency = 7,
    kUnexpectedCashResponse = 8,
};

template<class T>
class Expected {
    std::variant<T, ErrorCode> val_;

public:
    Expected(ErrorCode code) : val_{code} {} // NOLINT(google-explicit-constructor)

    Expected(T val) : val_{std::move(val)} {} // NOLINT(google-explicit-constructor)

    [[nodiscard]] bool hasError() const {
        return std::holds_alternative<ErrorCode>(val_);
    }

    [[nodiscard]] ErrorCode error() const noexcept {
        return std::get<ErrorCode>(val_);
    }

    [[nodiscard]]  T get() &{
        return std::get<T>(val_);
    }

    T get() &&{
        return std::move(std::get<T>(val_));
    }
};


using ExpectedVoid = Expected<void *>;
const ExpectedVoid NoErr = Expected<void *>(nullptr);

#endif //HIGHLOADCUP2021_ERROR_H
