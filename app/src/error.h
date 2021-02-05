#ifndef HIGHLOADCUP2021_ERROR_H
#define HIGHLOADCUP2021_ERROR_H

#include <variant>

enum class ErrorCode : int {
};

template<class T>
class Expected {
    std::variant<T, ErrorCode> val_;

public:
    explicit Expected(ErrorCode code) : val_{code} {}

    explicit Expected(T val) : val_{std::move(val)} {}

    explicit Expected(T &&val) : val_{val} {}

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


#endif //HIGHLOADCUP2021_ERROR_H
