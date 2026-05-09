#pragma once

#include <iomanip>
#include <iostream>
#include <string>

// Template class: the amount type can be double, int, float, and so on.
template <typename T>
class Transaction {
private:
    T amount;
    std::string date;
    std::string type;
    int relatedAccountId;

    static std::string encodeText(std::string text) {
        for (char& ch : text) {
            if (ch == ' ') {
                ch = '_';
            }
        }
        return text;
    }

    static std::string decodeText(std::string text) {
        for (char& ch : text) {
            if (ch == '_') {
                ch = ' ';
            }
        }
        return text;
    }

public:
    Transaction()
        : amount(0), date(""), type("Unknown"), relatedAccountId(-1) {
    }

    Transaction(T amount, const std::string& date, const std::string& type, int relatedAccountId = -1)
        : amount(amount), date(date), type(type), relatedAccountId(relatedAccountId) {
    }

    T getAmount() const {
        return amount;
    }

    std::string getDate() const {
        return date;
    }

    std::string getType() const {
        return type;
    }

    int getRelatedAccountId() const {
        return relatedAccountId;
    }

    // Serialization of one transaction into a compact text line.
    void writeToFile(std::ostream& os) const {
        os << encodeText(type) << ' '
           << std::fixed << std::setprecision(2) << amount << ' '
           << date << ' '
           << relatedAccountId << '\n';
    }

    bool readFromFile(std::istream& is) {
        std::string encodedType;
        if (!(is >> encodedType >> amount >> date >> relatedAccountId)) {
            return false;
        }

        type = decodeText(encodedType);
        return true;
    }

    // Operator overloading for pretty console output.
    friend std::ostream& operator<<(std::ostream& os, const Transaction<T>& transaction) {
        os << std::left << std::setw(12) << transaction.date << " | "
           << std::left << std::setw(16) << transaction.type << " | "
           << std::right << std::setw(10) << std::fixed << std::setprecision(2)
           << transaction.amount;

        if (transaction.relatedAccountId != -1) {
            os << " | related account: " << transaction.relatedAccountId;
        }

        return os;
    }
};
