#pragma once

#include "Account.h"

class CreditAccount : public Account {
private:
    double creditLimit;
    double interestRate;

protected:
    bool canWithdraw(double amount) const override;

public:
    CreditAccount(int id, const std::string& ownerName, double balance, double creditLimit, double interestRate);

    double getCreditLimit() const;
    double getInterestRate() const;
    std::string getType() const override;
    void deposit(double amount) override;
    void withdraw(double amount) override;
    void applyInterest() override;
    void print(std::ostream& os) const override;
    void serialize(std::ostream& os) const override;
};
