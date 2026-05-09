#include "CheckingAccount.h"
#include "Exceptions.h"

#include <iomanip>

CheckingAccount::CheckingAccount(int id, const std::string& ownerName, double balance, double overdraftLimit)
    : Account(id, ownerName, balance), overdraftLimit(overdraftLimit) {
}

bool CheckingAccount::canWithdraw(double amount) const {
    return amount > 0.0 && balance - amount >= -overdraftLimit;
}

double CheckingAccount::getOverdraftLimit() const {
    return overdraftLimit;
}

std::string CheckingAccount::getType() const {
    return "Checking";
}

void CheckingAccount::withdraw(double amount) {
    if (amount <= 0.0) {
        throw InvalidAmountException("Withdraw amount must be positive.");
    }

    if (!canWithdraw(amount)) {
        throw InsufficientFundsException("Checking account overdraft limit exceeded.");
    }

    changeBalance(-amount);
    addTransaction(Transaction<double>(amount, currentDate(), "Withdraw", -1));
}

void CheckingAccount::applyInterest() {
    // Checking account has no interest, but the virtual method must be implemented.
}

void CheckingAccount::print(std::ostream& os) const {
    Account::print(os);
    os << ", Overdraft limit: " << std::fixed << std::setprecision(2) << overdraftLimit;
}

void CheckingAccount::serialize(std::ostream& os) const {
    os << "ACCOUNT Checking\n";
    os << "ID " << id << '\n';
    os << "OWNER " << std::quoted(ownerName) << '\n';
    os << "BALANCE " << std::fixed << std::setprecision(2) << balance << '\n';
    os << "OVERDRAFT_LIMIT " << overdraftLimit << '\n';
    os << "TRANSACTIONS " << history.size() << '\n';

    for (const Transaction<double>& transaction : history) {
        transaction.writeToFile(os);
    }

    os << "END_ACCOUNT\n";
}
