#pragma once

#include "Account.h"

class SavingsAccount : public Account {
private:
    double interestRate;

protected:
    bool canWithdraw(double amount) const override;

public:
    SavingsAccount(int id, const std::string& ownerName, double balance, double interestRate);

    double getInterestRate() const;
    std::string getType() const override;
    void withdraw(double amount) override;
    void applyInterest() override;
    void print(std::ostream& os) const override;
    void serialize(std::ostream& os) const override;
};
