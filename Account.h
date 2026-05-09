#pragma once

#include "Transaction.h"

#include <iostream>
#include <string>
#include <vector>

class Bank;

// Abstract base class. It defines the common interface for all account types.
class Account {
    friend class Bank;

protected:
    int id;
    std::string ownerName;
    double balance;
    std::vector<Transaction<double>> history;

    static std::string currentDate();
    void changeBalance(double amount);
    virtual bool canWithdraw(double amount) const = 0;

public:
    Account(int id, const std::string& ownerName, double balance);
    virtual ~Account() = default;

    int getId() const;
    std::string getOwnerName() const;
    double getBalance() const;
    const std::vector<Transaction<double>>& getHistory() const;

    virtual std::string getType() const = 0;
    virtual void deposit(double amount);
    virtual void withdraw(double amount) = 0;
    virtual void applyInterest() = 0;

    void addTransaction(const Transaction<double>& transaction);

    // Virtual method: polymorphism lets derived classes print extra fields.
    virtual void print(std::ostream& os) const;

    // Virtual serialization: each derived class writes its own extra fields.
    virtual void serialize(std::ostream& os) const = 0;
};

// Operator overloading: printing an Account calls the virtual print method.
std::ostream& operator<<(std::ostream& os, const Account& account);
