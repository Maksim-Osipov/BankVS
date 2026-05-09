#pragma once

#include <stdexcept>
#include <string>

// Custom exceptions demonstrate inheritance from a standard exception class.
class InsufficientFundsException : public std::runtime_error {
public:
    explicit InsufficientFundsException(const std::string& message)
        : std::runtime_error(message) {
    }
};

class InvalidAccountException : public std::runtime_error {
public:
    explicit InvalidAccountException(const std::string& message)
        : std::runtime_error(message) {
    }
};

class InvalidAmountException : public std::runtime_error {
public:
    explicit InvalidAmountException(const std::string& message)
        : std::runtime_error(message) {
    }
};
