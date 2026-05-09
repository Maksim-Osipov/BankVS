#pragma once

#include "Account.h"

class CheckingAccount : public Account {
private:
    double overdraftLimit;

protected:
    bool canWithdraw(double amount) const override;

public:
    CheckingAccount(int id, const std::string& ownerName, double balance, double overdraftLimit);

    double getOverdraftLimit() const;
    std::string getType() const override;
    void withdraw(double amount) override;
    void applyInterest() override;
    void print(std::ostream& os) const override;
    void serialize(std::ostream& os) const override;
};
