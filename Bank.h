#pragma once

#include "Account.h"

#include <iosfwd>
#include <map>
#include <memory>
#include <string>

struct TransferRequest {
    int fromId;
    int toId;
    double amount;
};

class Bank {
private:
    // Smart pointers provide shared ownership of polymorphic Account objects.
    std::map<int, std::shared_ptr<Account>> accounts;

    void validateNewAccount(int id) const;

public:
    void createSavingsAccount(int id, const std::string& ownerName, double initialBalance, double interestRate);
    void createCheckingAccount(int id, const std::string& ownerName, double initialBalance, double overdraftLimit);
    void createCreditAccount(int id, const std::string& ownerName, double initialBalance, double creditLimit, double interestRate);

    std::shared_ptr<Account> getAccount(int id);
    void removeAccount(int id);
    void deposit(int id, double amount);
    void withdraw(int id, double amount);
    void transfer(int fromId, int toId, double amount);
    void applyInterestToAccount(int id);
    void applyInterestToAllAccounts();
    void printAllAccounts() const;
    void printAccountHistory(int id) const;
    std::string getAllAccountsText() const;
    std::string getAccountHistoryText(int id) const;
    void generateStatement(int id, const std::string& filename) const;
    const std::map<int, std::shared_ptr<Account>>& getAccounts() const;
    void clear();

    // Operator overloading: transfer can be written as bank + TransferRequest{...}.
    Bank& operator+(const TransferRequest& request);

    friend std::ostream& operator<<(std::ostream& os, const Bank& bank);
    friend std::istream& operator>>(std::istream& is, Bank& bank);
};

std::ostream& operator<<(std::ostream& os, const Bank& bank);
std::istream& operator>>(std::istream& is, Bank& bank);
