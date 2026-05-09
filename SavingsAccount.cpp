#include "SavingsAccount.h"
#include "Exceptions.h"

#include <iomanip>

SavingsAccount::SavingsAccount(int id, const std::string& ownerName, double balance, double interestRate)
    : Account(id, ownerName, balance), interestRate(interestRate) {
}

bool SavingsAccount::canWithdraw(double amount) const {
    return amount > 0.0 && balance >= amount;
}

double SavingsAccount::getInterestRate() const {
    return interestRate;
}

std::string SavingsAccount::getType() const {
    return "Savings";
}

void SavingsAccount::withdraw(double amount) {
    if (amount <= 0.0) {
        throw InvalidAmountException("Withdraw amount must be positive.");
    }

    if (!canWithdraw(amount)) {
        throw InsufficientFundsException("Savings account cannot have a negative balance.");
    }

    changeBalance(-amount);
    addTransaction(Transaction<double>(amount, currentDate(), "Withdraw", -1));
}

void SavingsAccount::applyInterest() {
    if (balance > 0.0) {
        const double interest = balance * interestRate;
        changeBalance(interest);
        addTransaction(Transaction<double>(interest, currentDate(), "Interest", -1));
    }
}

void SavingsAccount::print(std::ostream& os) const {
    Account::print(os);
    os << ", Interest rate: " << std::fixed << std::setprecision(2) << interestRate * 100.0 << "%";
}

void SavingsAccount::serialize(std::ostream& os) const {
    os << "ACCOUNT Savings\n";
    os << "ID " << id << '\n';
    os << "OWNER " << std::quoted(ownerName) << '\n';
    os << "BALANCE " << std::fixed << std::setprecision(2) << balance << '\n';
    os << "INTEREST_RATE " << interestRate << '\n';
    os << "TRANSACTIONS " << history.size() << '\n';

    for (const Transaction<double>& transaction : history) {
        transaction.writeToFile(os);
    }

    os << "END_ACCOUNT\n";
}
