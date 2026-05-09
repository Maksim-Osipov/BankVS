#include "Account.h"
#include "Exceptions.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

Account::Account(int id, const std::string& ownerName, double balance)
    : id(id), ownerName(ownerName), balance(balance) {
}

std::string Account::currentDate() {
    const std::time_t now = std::time(nullptr);
    std::tm localTime {};

#ifdef _WIN32
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif

    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d");
    return oss.str();
}

void Account::changeBalance(double amount) {
    balance += amount;
}

int Account::getId() const {
    return id;
}

std::string Account::getOwnerName() const {
    return ownerName;
}

double Account::getBalance() const {
    return balance;
}

const std::vector<Transaction<double>>& Account::getHistory() const {
    return history;
}

void Account::deposit(double amount) {
    if (amount <= 0.0) {
        throw InvalidAmountException("Deposit amount must be positive.");
    }

    changeBalance(amount);
    addTransaction(Transaction<double>(amount, currentDate(), "Deposit", -1));
}

void Account::addTransaction(const Transaction<double>& transaction) {
    history.push_back(transaction);
}

void Account::print(std::ostream& os) const {
    os << "ID: " << id
       << ", Type: " << getType()
       << ", Owner: " << ownerName
       << ", Balance: " << std::fixed << std::setprecision(2) << balance;
}

std::ostream& operator<<(std::ostream& os, const Account& account) {
    account.print(os);
    return os;
}
